//
//  Sampler.hpp
//  
//
//  Created by Billy James on 03/03/26.
//

#pragma once

namespace sketchbook
{

class BandLimitedWaveTable : public Module::SharedData
{
public:
    
    struct WaveTableRow
    {
        juce::AudioBuffer<float> data;
        int maxMidiNote = 127;
    };
    
    /// note - due to templat
    BandLimitedWaveTable()
    {
        
    }
    
    void setSample(juce::String ident, juce::AudioBuffer<float>& sample, float sampleRateOfSample)
    {
        if (m_currSampleIdent != ident)
        {
            m_currSampleIdent = ident;
            m_originalSampleRate = sampleRateOfSample;
            m_originalSample.makeCopyOf(sample);
            calculateRows();
        }
    }
    
    void setSampleRate(float samplerate)
    {
        if (m_samplerate != samplerate)
        {
            m_samplerate = samplerate;
            calculateRows();
        }
    }
    
    std::shared_ptr<WaveTableRow> getRowForMidiNote(int midiNote)
    {
        for (int i = m_rows.size(); i > 0; i--)
        {
            if (midiNote <= m_rows[i-1]->maxMidiNote)
                return m_rows[i-1];
        }

        // Fallback: return the last row (highest band)
        return m_rows[m_rows.size() - 1];
    }
    
    /// a function to swap out the samples that should only be called on the audio thread
    void swapOutRowsIfNeeded()
    {
        if (m_waitingToSwapOutRows)
        {
            m_rows.clear();
            m_rows.addArray(m_waitingRows);
            m_waitingRows.clear();
            m_waitingToSwapOutRows = false;
        }
    }
    
    float getSampleRateOfSample()
    {
        return m_originalSampleRate;
    }
    
    bool hasSample()
    {
        return m_currSampleIdent.isNotEmpty();
    }
    
private:
    
    void calculateRows()
    {
        // The input sample is automaticaly place at c4
        const int baseMidiNote = 69;
        const int notesPerTable = (128 - baseMidiNote) / m_numWaveTables;
        const float nyquist = m_originalSampleRate * 0.5f;
        const float sampleRateRatio = m_samplerate / m_originalSampleRate;
        m_waitingRows.clear();
          
        for (int i = 0; i < m_numWaveTables; i++)
        {
            //alocate and setup row
            auto row = std::make_shared<WaveTableRow>();
            row->maxMidiNote = baseMidiNote + notesPerTable * i;
            row->data.makeCopyOf(m_originalSample);
            m_waitingRows.add(row);
            
            //calculate low pass cuttoff
            float octavesFromBase = (row->maxMidiNote - baseMidiNote) / 12;
            float cuttoff = std::clamp(nyquist * sampleRateRatio * std::pow(2.0f, -octavesFromBase), 10.0f, nyquist);
            m_lowPass.setCoefficients(juce::IIRCoefficients::makeLowPass(m_originalSampleRate, cuttoff));

            //perform cutoff
            for (int ch = 0; ch < row->data.getNumChannels(); ch++)
            {
                m_lowPass.reset();
                auto* channelData = row->data.getWritePointer(ch);
                m_lowPass.processSamples(channelData, row->data.getNumSamples());
            }
        }
        
        m_waitingToSwapOutRows = true;
    }
    
    float m_samplerate = 44100;
    float m_originalSampleRate= 44100;
    const int m_numWaveTables = 16;
    bool m_waitingToSwapOutRows = false;
    juce::AudioBuffer<float> m_originalSample;
    juce::Array<std::shared_ptr<WaveTableRow>> m_rows;
    juce::Array<std::shared_ptr<WaveTableRow>> m_waitingRows;
    juce::IIRFilter m_lowPass;
    juce::String m_currSampleIdent;
};

class Sampler : public Module, public Module::SharedDataHolder<BandLimitedWaveTable>
{
public:
    
    Sampler()
    {
        setVoiceMonitorType(adsr);
        
        m_audioFormatManager.registerBasicFormats();
        
        //set the parameter structure
        setModuleParameters(
        {
            Parameter::File("Selected Sample", [&] (juce::String filePath)
            {
                if (!juce::File::isAbsolutePath(filePath) || !juce::File(filePath).exists())
                    return;
                    
                //attempt to load audio from the file
                juce::AudioBuffer<float> buffer;
                if (auto* reader = m_audioFormatManager.createReaderFor(juce::File(filePath)))
                {
                    buffer.setSize(reader->numChannels, int(reader->lengthInSamples));
                    if (reader->read(&buffer, 0, int(reader->lengthInSamples), 0, true, true))
                    {
                        getSharedData<BandLimitedWaveTable>()->setSample(filePath, buffer, reader->sampleRate);
                    }
                }
            }, "Grand Piano"),
            
            Parameter::Boolean("Resample To Pitch", [this] (bool value)
                               {
                DBG(juce::String("Resample To Pitch : ") + (value ? juce::String("True") : juce::String("False")));
            }, true),
        });
    }
    
    ~Sampler()
    {
        
    }
    
    void prepareToPlay(float samplerate, int buffersize) override
    {
        m_samplerate = samplerate;
        getSharedData<BandLimitedWaveTable>()->setSampleRate(samplerate);
        
        if (!getSharedData<BandLimitedWaveTable>()->hasSample())
        {
            //load initial audio sample from binary data
            juce::AudioBuffer<float> buffer;
            
            if (auto* reader = m_audioFormatManager.createReaderFor(std::make_unique<juce::MemoryInputStream>(DSP_SKETCHBOOK_BINARY::grand_piano_C_wav,
                                                                                                              DSP_SKETCHBOOK_BINARY::grand_piano_C_wavSize,
                                                                                                              false)))
            {
                buffer.setSize(reader->numChannels, int(reader->lengthInSamples));
                if (reader->read(&buffer, 0, int(reader->lengthInSamples), 0, true, true))
                {
                    getSharedData<BandLimitedWaveTable>()->setSample("Grand Piano", buffer, reader->sampleRate);
                }
            }
        }
    }
    
    void noteOn(const sketchbook::NoteOnEvent& event) override
    {
        if (!event.isLegatoNoteOn)
        {
            reset();
            m_midiNote = event.midiMessage.getNoteNumber();
            getSharedData<BandLimitedWaveTable>()->swapOutRowsIfNeeded();
            m_currSample = getSharedData<BandLimitedWaveTable>()->getRowForMidiNote(m_midiNote);
        }
    }
    
    void noteOff(bool) override
    {
        return;
    }
    
    void reset() override
    {
        m_index = 0.f;
        return;
    }
    
    void pitchUpdated(float pitchHz) override
    {
        if (pitchHz != m_freqHz)
        {
            m_freqHz = pitchHz;
            calcIncrement();
        }
    }
    
    void processSample(float* sample) override
    {
        if (!m_currSample) return;
        
        float l = m_currSample->data.getWritePointer(0)[int(m_index)];
        float r = m_currSample->data.getWritePointer(0)[int(m_index)+1];
        float frac = m_index - int(m_index);
        *sample += linearInterp(l, r, frac);
        
        //no looping so just get rid of the sample
        m_index += m_increment;
        if (m_index >= m_currSample->data.getNumSamples()-1)
            m_currSample.reset();
    }
    
    juce::String getName() override
    {
        return "Sampler";
    }
    
private:
    
    inline float linearInterp(float l, float r, float frac)
    {
        return l + frac * (r - l);
    }
    
    void calcIncrement()
    {
        m_increment = (m_freqHz / 440) * (getSharedData<BandLimitedWaveTable>()->getSampleRateOfSample() / m_samplerate);
    }
    
private:
    float m_samplerate = 44100;
    
    //sample playback
    int m_midiNote=60;
    float m_freqHz=440.f;
    float m_index = 0.f;
    float m_increment = 0.f;
    
    //wave tables
    std::shared_ptr<BandLimitedWaveTable::WaveTableRow> m_currSample;
    juce::AudioFormatManager m_audioFormatManager;
};
} // end namespace sketchbook
