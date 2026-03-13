/*
  ==============================================================================

    ParamaterPages.h
    Created: 8 May 2025 3:26:41pm
    Author:  William James

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Slider.h"

//==============================================================================
/*
*/
namespace sketchbook
{

class MatrixMenuButton : public juce::TextButton
{
    class MatrixMenuPopup : public juce::Component
    {
        public:
        MatrixMenuPopup(juce::StringArray options)
        {
            for (auto str : options)
            {
                auto row = new juce::Label();
                addChildComponent(row);
                row->addMouseListener(this, true);
                row->setText(str, juce::dontSendNotification);
                row->setJustificationType(juce::Justification::centred);
                row->setColour(juce::Label::textColourId, Style::getInstance()->themeColour);
                row->setFont(Style::getInstance()->themeFont.withHeight(10));
                rows.add(row);
            }
                
            animatorUpdater.addAnimator(sizeAnimator);
            sizeAnimator.start();
        }
        
        ~MatrixMenuPopup()
        {
            for (auto r : rows)
            {
                if (r)
                {
                    delete r;
                    r = nullptr;
                }
            }
        }
        
        void setInitialSize(juce::Rectangle<int> size, float cornerRadius)
        {
            m_initialSize = size;
            m_initialCornerRadius = cornerRadius;
        }
        
        void paint(juce::Graphics& g) override
        {
            float x      = getLocalBounds().getX() + (getWidth() - m_initialSize.getWidth()) * (1.f - m_animProgress01);
            float width  = m_initialSize.getWidth() + (getWidth() - m_initialSize.getWidth()) * m_animProgress01;
            float height = m_initialSize.getHeight() + (getHeight() - m_initialSize.getHeight()) * m_animProgress01;
            float corner = m_initialCornerRadius + (m_expandedCornerRadius - m_initialCornerRadius) * m_animProgress01;
            juce::Rectangle<float> shape(x, 0, width, height);
            g.setColour(Style::getInstance()->backgroundColour);
            g.fillRoundedRectangle(shape, corner);
            
            g.setColour(Style::getInstance()->themeColour);
            g.drawRoundedRectangle(shape.reduced(0.5f), corner, 1.f);
        }
        
        void resized() override
        {
            auto area = getLocalBounds();
            
            area.removeFromTop(RowHeight / 2);
            for (auto r : rows)
                r->setBounds(area.removeFromTop(RowHeight));
        }
        
        std::function<void(juce::String)> optionSelected;
        static const int RowHeight = 18;
        
        private:
        
        void mouseEnter(const juce::MouseEvent& event) override
        {
            for (auto r : rows)
            {
                if (event.eventComponent == r)
                {
                    r->setColour(juce::Label::textColourId, Style::getInstance()->highlightColour);
                    r->repaint();
                }
            }
        }
        
        void mouseExit(const juce::MouseEvent& event) override
        {
            for (auto r : rows)
            {
                if (event.eventComponent == r)
                {
                    r->setColour(juce::Label::textColourId, Style::getInstance()->themeColour);
                    r->repaint();
                }
            }
        }
        
        void mouseUp(const juce::MouseEvent& event) override
        {
            if (!event.eventComponent->contains(event.getPosition()))
                return;
            
            for (auto r : rows)
            {
                if (event.eventComponent == r)
                {
                    optionSelected(r->getText());
                    juce::ModalComponentManager::getInstance()->cancelAllModalComponents();
                }
            }
        }
        
        juce::VBlankAnimatorUpdater animatorUpdater {this};
        juce::Animator sizeAnimator = juce::ValueAnimatorBuilder()
            .withEasing(juce::Easings::createEaseIn())
            .withDurationMs (130)
            .withValueChangedCallback ([this] (float value)
            {
                m_animProgress01 = value;
                repaint();
            })
            .withOnCompleteCallback([this] ()
            {
                for (auto r : rows)
                    r->setVisible(true);
            })
            .build();
        
        juce::Rectangle<int> m_initialSize;
        float m_initialCornerRadius;
        float m_expandedCornerRadius=3.5;
        float m_animProgress01=0;
        juce::Array<juce::Label*> rows;
    };
    
    public:
    
    MatrixMenuButton()
    {
        setButtonText("Mod");
        setClickingTogglesState(false);
        setInterceptsMouseClicks(true, true);
        setTriggeredOnMouseDown(true);
    }
    
    void setData(juce::ValueTree newData)
    {
        data = newData;
    }
    
    private:
    
    juce::StringArray getModulationSourceNames()
    {
        auto sources = data.getRoot().getChildWithName(Module::ParamIdents::MODULATION_SOURCES);
        if (!sources.isValid()) return {};
        
        juce::StringArray output;
        for (auto sourceModule : sources)
            output.add(sourceModule[Module::ParamIdents::NAME].toString().replace("_", " "));
        
        return output;
    }
    
    void clicked() override
    {
        //add the menu as a popup
        const auto optionsList = getModulationSourceNames();
        const juce::Rectangle<int> size = {100,
                                           MatrixMenuPopup::RowHeight * (optionsList.size() + 1)}; //plus one for spacing top and bottom
        
        auto popup = std::make_unique<MatrixMenuPopup>(optionsList);
        popup->setName("Matrix Menu");
        popup->setSize(size.getWidth(), size.getHeight());
        popup->setInitialSize(getLocalBounds(), getHeight() / 2);
        popup->optionSelected = [sp = SafePointer<MatrixMenuButton>(this)] (juce::String selection)
        {
           sp->data.addChild(Module::ModifiedParameter::defaultMappingTo(selection.replace(" ", "_")), -1, nullptr);
        };
        
        auto calloutPos = size.withPosition(getScreenBounds().getRight() - size.getWidth(), getScreenBounds().getY());
        auto& callout = juce::CallOutBox::launchAsynchronously(std::move(popup),
                                                              size,
                                                              nullptr);
        callout.setLookAndFeel(&getLookAndFeel());
        callout.setArrowSize(0);
        callout.updatePosition(size, calloutPos);
    }
    
    private:
    juce::ValueTree data;
};

class OnOffButton : public juce::TextButton
{
    public:
    OnOffButton()
    {
        setButtonText(getToggleState() ? m_onText : m_offText);
    }
    
    void setOnOffText(juce::String onText, juce::String offText)
    {
        m_onText = onText;
        m_offText = offText;
        setButtonText(getToggleState() ? m_onText : m_offText);
    }
    
    void buttonStateChanged() override
    {
        setButtonText(getToggleState() ? m_onText : m_offText);
        repaint();
    }
    
    private:
    juce::String m_onText = "On";
    juce::String m_offText = "Off";
};

class ExpandableListBox : public juce::Viewport
{
    public:
    class ExpandableListItem : public juce::Component
    {
        public:
        virtual int getFullHeight()=0;
    };
    
    class ListItemHolder : public juce::Component
    {
        public:
        class Header : public juce::Component
        {
            public:
            
            Header(juce::String titleText)
            {
                addAndMakeVisible(titleLabel);
                titleLabel.setJustificationType(juce::Justification::centred);
                titleLabel.setFont(Style::getInstance()->themeFont.withHeight(14.f));
                titleLabel.setColour(juce::Label::ColourIds::textColourId, Style::getInstance()->themeColour);
                titleLabel.setText(titleText.replace("_", " "), juce::dontSendNotification);
                
                arrowIcon.reset(juce::Drawable::createFromImageData(DSP_SKETCHBOOK_BINARY::arrowdown_svg,
                                                                    DSP_SKETCHBOOK_BINARY::arrowdown_svgSize).release());
                arrowIcon->replaceColour(juce::Colours::white, Style::getInstance()->themeColour);
                
                animatorUpdater.addAnimator(arrowAnimator);
            }
            
            void resized() override
            {
                auto area = getLocalBounds();
                titleLabel.setBounds(area);
                arrowIcon->setBounds(area.removeFromRight(Height * 0.9f).removeFromLeft(Height * 0.5));
            }
            
            void paint(juce::Graphics& g) override
            {
                /// experamentaly found pivit point - how should this work?
                arrowIcon->setTransform(juce::AffineTransform::rotation(arrowAngleRadians, arrowIcon->getWidth() * 0.42f, arrowIcon->getHeight() * 0.2f));
                arrowIcon->drawWithin(g, arrowIcon->getBounds().toFloat(), juce::RectanglePlacement::centred, 1.0);
            }
            
            void startAnimation()
            {
                arrowAnimator.start();
            }
            
            juce::Component& getTitleComp()
            {
                return titleLabel;
            }
            
            juce::Component& getArrowComp()
            {
                return *arrowIcon.get();
            }
            
            static const int Height = 40;
            
            private:
            
            juce::Label titleLabel;
            std::unique_ptr<juce::Drawable> arrowIcon;
            
            //for dropdown animation
            float arrowAngleRadians = 0.0f;
            const float maxAngleRadians = juce::MathConstants<float>::pi;
            bool isOpen = false;
            
            juce::VBlankAnimatorUpdater animatorUpdater {this};
            juce::Animator arrowAnimator = juce::ValueAnimatorBuilder()
                .withEasing(juce::Easings::createLinear())
                .withDurationMs (150)
                .withValueChangedCallback ([this] (float value)
                {
                    arrowAngleRadians = (isOpen ? 1.f - value : value) * maxAngleRadians;
                    repaint();
                })
                .withOnCompleteCallback([this] ()
                {
                    isOpen = !isOpen;
                })
                .build();
        };
        
        ListItemHolder(ExpandableListItem* content, juce::String headerTitle, std::unique_ptr<Header> headerComp = nullptr)
        : m_content(content)
        {
            jassert(content);
            addAndMakeVisible(m_content);
            
            if (headerComp)
                m_header = std::move(headerComp);
            else
                m_header.reset(new Header(headerTitle));
            
            addAndMakeVisible(m_header.get());
            m_header->addMouseListener(this, true);
            
            animatorUpdater.addAnimator(dropDownAnimator);
        }
        
        void resized() override
        {
            auto area = getLocalBounds();
            
            m_header->setBounds(area.removeFromTop(ListItemHolder::Header::Height));
            if (m_content)
                m_content->setBounds(area);
        }
        
        int getCurrentHeight()
        {
            return Header::Height + currentContentHeight;
        }
        
        void mouseUp(const juce::MouseEvent& event) override
        {
            if (event.eventComponent == m_header.get()
                || event.eventComponent == &m_header->getTitleComp()
                || event.eventComponent == &m_header->getArrowComp())
            {
                dropDownAnimator.start();
                m_header->startAnimation();
            }
        }
        
        void paint(juce::Graphics& g) override
        {
            g.setColour(Style::getInstance()->themeColour);
            g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.5f), 3.5, 1.f);
        }
        
        std::function<void()> onSizeChange;
        
        private:
        
        bool isOpen = false;
        int currentContentHeight = 0;
        
        std::unique_ptr<Header> m_header;
        ExpandableListItem* m_content = nullptr;
        
        //for dropdown animation
        juce::VBlankAnimatorUpdater animatorUpdater {this};
        juce::Animator dropDownAnimator = juce::ValueAnimatorBuilder()
            .withEasing(juce::Easings::createEaseInOutCubic())
            .withDurationMs (150)
            .withValueChangedCallback ([this] (float value)
                                       {
                float multiple = isOpen ? 1.0f - value : value;
                currentContentHeight = m_content->getFullHeight() * multiple;
                
                if (onSizeChange)
                    onSizeChange();
            })
            .withOnCompleteCallback([this] ()
                                    {
                isOpen = !isOpen;
            })
            .build();
    };
    
    class ListHolder : public juce::Component
    {
        public:
        
        ListHolder()
        {
            
        }
        
        ~ListHolder()
        {
            for (auto item : m_items)
            {
                if (item)
                {
                    delete item;
                    item=nullptr;
                }
            }
        }
        
        void resized() override
        {
            auto area = getLocalBounds();
            for (auto item : m_items)
            {
                item->setBounds(area.removeFromTop(item->getCurrentHeight()));
                area.removeFromTop(padding);
            }
        }
        
        
        void addItem(ExpandableListItem* item, juce::String headerText, std::unique_ptr<ExpandableListBox::ListItemHolder::Header> headerComp = nullptr)
        {
            auto itemHolder = new ListItemHolder(item, headerText, std::move(headerComp));
            m_items.add(itemHolder);
            addAndMakeVisible(itemHolder);
            itemHolder->onSizeChange = [this] ()
            {
                setSize(getWidth(), calcHeight());
            };
            setSize(getWidth(), calcHeight());
        }
        
        int calcHeight()
        {
            int h = 0;
            for (auto& item : m_items)
                h += item->getCurrentHeight() + padding;
            
            return h;
        }
        
        juce::Array<ListItemHolder*> m_items;
        const int padding = 6;
    };
    
    ExpandableListBox()
    {
        setViewedComponent(&listHolder);
        addAndMakeVisible(&listHolder);
        setScrollBarsShown(true, false);
        getVerticalScrollBar().setAutoHide(true);
        setScrollBarThickness(7);
    }
    
    void addItem(ExpandableListItem* item, juce::String headerText, std::unique_ptr<ExpandableListBox::ListItemHolder::Header> headerComp = nullptr)
    {
        listHolder.addItem(item, headerText, std::move(headerComp));
    }
    
    void resized() override
    {
        listHolder.setBounds(juce::Rectangle<int>(getScrollBarThickness(), 0, getWidth() - getScrollBarThickness(), listHolder.getHeight()));
    }
    
    ListHolder listHolder;
};

class ModuleGroupPage : public ExpandableListBox, public juce::ValueTree::Listener
{
    
    class ParameterRow : public juce::Component, public juce::ValueTree::Listener
    {
        public:
        ParameterRow(Context& ctx)
        : slider(ctx)
        {
            addAndMakeVisible   (parameterTitleLabel);
            parameterTitleLabel.setJustificationType(juce::Justification::centredLeft);
            parameterTitleLabel.setFont(Style::getInstance()->themeFont.withHeight(12));
            parameterTitleLabel.setColour(juce::Label::ColourIds::textColourId, Style::getInstance()->themeColour);
            
            addChildComponent(sliderLabel);
            addChildComponent(slider);
            addChildComponent(integerStepper);
            addChildComponent(toggleButton);
            addChildComponent(comboOptions);
            addAndMakeVisible(addModButton);
            addChildComponent(browserButton);
            addChildComponent(selectedFileLabel);
        }
        
        ~ParameterRow() override
        {
            if (data.isValid())
                data.removeListener (this);
        }
        
        void setData (juce::ValueTree _data)
        {
            if (!_data.isValid())
                return;
            
            if (data.isValid())
                data.removeListener (this);
            
            data = _data;
            data.addListener (this);
            
            switch (getParamType(data))
            {
                case Module::parameterType::floatParam:
                {
                    slider.setRange(data[Module::ParamIdents::MIN], data[Module::ParamIdents::MAX]);
                    slider.setData(data);
                    slider.setValue(data[Module::ParamIdents::VALUE], juce::sendNotification);
                    slider.onValueChange = [this]
                    {
                        data.setPropertyExcludingListener(this, Module::ParamIdents::VALUE, slider.getValue(), nullptr);
                        sliderLabel.setText(juce::String(std::round(slider.getValue() * 1000) / 1000), juce::dontSendNotification);
                    };
                    sliderLabel.setJustificationType(juce::Justification::centred);
                    sliderLabel.setFont(Style::getInstance()->themeFont.withHeight(10));
                    break;
                }
                case Module::parameterType::intParam:
                {
                    integerStepper.setSliderStyle(juce::Slider::IncDecButtons);
                    integerStepper.setRange(data[Module::ParamIdents::MIN], data[Module::ParamIdents::MAX]);
                    integerStepper.setValue(data[Module::ParamIdents::VALUE], juce::sendNotification);
                    integerStepper.onValueChange = [this]
                    {
                        data.setPropertyExcludingListener(this, Module::ParamIdents::VALUE, integerStepper.getValue(), nullptr);
                    };
                    break;
                }
                case Module::parameterType::booleanParam:
                {
                    toggleButton.setToggleState(data[Module::ParamIdents::VALUE], juce::sendNotification);
                    toggleButton.setClickingTogglesState (true);
                    toggleButton.onClick = [this]
                    {
                        data.setPropertyExcludingListener(this, Module::ParamIdents::VALUE, toggleButton.getToggleState(), nullptr);
                    };
                    break;
                }
                case Module::parameterType::choiceParam:
                {
                    comboOptions.clear();
                    juce::StringArray options = juce::StringArray::fromTokens(data[Module::ParamIdents::PARAMETER_OPTIONS].toString(), ";", "");
                    comboOptions.addItemList(options, 1);
                    comboOptions.setSelectedItemIndex(options.indexOf(data[Module::ParamIdents::VALUE].toString()), juce::sendNotification);
                    comboOptions.onChange = [this, options]
                    {
                        data.setPropertyExcludingListener(this, Module::ParamIdents::VALUE, options[comboOptions.getSelectedItemIndex()], nullptr);
                    };
                    break;
                }
                case Module::parameterType::fileParam:
                {
                    selectedFileLabel.setJustificationType(juce::Justification::centred);
                    selectedFileLabel.setText(data[Module::ParamIdents::VALUE], juce::dontSendNotification);
                    browserButton.setButtonText("Open File");
                    browserButton.setClickingTogglesState(false);
                    browserButton.onClick = [this] ()
                    {
                        //allow user to choose file
                        fileChooser = std::make_unique<juce::FileChooser>("Open File", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory));
                        
                        auto folderChooserFlags = juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::openMode;
                        fileChooser->launchAsync (folderChooserFlags, [sp = SafePointer<ParameterRow>(this)] (const juce::FileChooser& chooser)
                        {
                            if (!sp) return;
                            
                            //get result and check extention
                            juce::File path = chooser.getResult();
                            
                            if (path == juce::File())
                                return;
                            
                            sp->selectedFileLabel.setText(path.getFileName(), juce::dontSendNotification);
                            sp->data.setPropertyExcludingListener(sp.getComponent(), Module::ParamIdents::VALUE, path.getFullPathName(), nullptr);
                        });
                    };
                    
                }
                case Module::parameterType::numParameterTypes:
                default: break;
            };
            
            setControlVisibilityForParamType();
            parameterTitleLabel.setText (data[Module::ParamIdents::PARAMETER_NAME], juce::dontSendNotification);
            
            addModButton.setData(data);
        }
        
        void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override
        {
            if (property != Module::ParamIdents::VALUE) return;
            
            switch (getParamType(data))
            {
                case Module::parameterType::floatParam:
                {
                    slider.setValue(data[Module::ParamIdents::VALUE], juce::dontSendNotification);
                    break;
                }
                case Module::parameterType::intParam:
                {
                    integerStepper.setValue(data[Module::ParamIdents::VALUE], juce::dontSendNotification);
                    break;
                }
                case Module::parameterType::booleanParam:
                {
                    toggleButton.setToggleState(data[Module::ParamIdents::VALUE], juce::dontSendNotification);
                    break;
                }
                case Module::parameterType::choiceParam:
                {
                    juce::StringArray options = juce::StringArray::fromTokens(data[Module::ParamIdents::PARAMETER_OPTIONS].toString(), ";", "");
                    comboOptions.setSelectedItemIndex(options.indexOf(data[Module::ParamIdents::VALUE].toString()), juce::dontSendNotification);
                    break;
                }
                default: break;
            };
        }
        
        void resized() override
        {
            auto area = getLocalBounds().reduced(int(getWidth() * 0.03f), 0);
            int w = area.getWidth() / 3;
            parameterTitleLabel.setBounds (area.removeFromLeft (w).reduced(3));
            
            if (addModButton.isVisible())
            {
                addModButton.setBounds(area.removeFromRight(int(getWidth() * 0.11)).reduced(0, getHeight() * 0.25));
            }
            
            //centred slider
            const auto sliderRect       = juce::Rectangle<int>(getWidth() / 4, getHeight());
            const auto sliderLabelRect  = juce::Rectangle<int>(int(getWidth() * 0.11), getHeight());
            slider         .setBounds(sliderRect.withCentre(getLocalBounds().getCentre()));
            sliderLabel    .setBounds(sliderLabelRect.withX(slider.getRight()));
            
            //other widgets to the right
            integerStepper .setBounds(juce::Rectangle<int>(area).removeFromRight(w / 1.5).reduced(0, getHeight() * 0.24));
            toggleButton   .setBounds(juce::Rectangle<int>(area).removeFromRight(w / 1.5).reduced(0, getHeight() * 0.24));
            comboOptions   .setBounds(juce::Rectangle<int>(area).removeFromRight(w / 1.5).reduced(0, getHeight() * 0.24));
            
            //file component
            browserButton    .setBounds(area.removeFromRight(w / 1.5).reduced(0, getHeight() * 0.24));
            selectedFileLabel.setBounds(area);
        }
        
        static const int getRowHeight()
        {
            return 48;
        }
        
        private:
        
        Module::parameterType getParamType(juce::ValueTree parameterData)
        {
            if (parameterData.getType() == Module::ParamIdents::PARAMETER_FLOAT)
            {
                return Module::parameterType::floatParam;
            }
            else if (parameterData.getType() == Module::ParamIdents::PARAMETER_INTEGER)
            {
                return Module::parameterType::intParam;
            }
            else if (parameterData.getType() == Module::ParamIdents::PARAMETER_BOOL)
            {
                return Module::parameterType::booleanParam;
            }
            else if (parameterData.getType() == Module::ParamIdents::PARAMETER_CHOICE)
            {
                return Module::parameterType::choiceParam;
            }
            else if (parameterData.getType() == Module::ParamIdents::PARAMETER_FILE)
            {
                return Module::parameterType::fileParam;
            }
            
            //default to floating point (but throw assert here anyway)
            jassertfalse;
            return Module::parameterType::floatParam;
        }
        
        void setControlVisibilityForParamType()
        {
            auto type = getParamType(data);
            addModButton     .setVisible(type == Module::parameterType::floatParam);
            slider           .setVisible(type == Module::parameterType::floatParam);
            sliderLabel      .setVisible(type == Module::parameterType::floatParam);
            integerStepper   .setVisible(type == Module::parameterType::intParam);
            toggleButton     .setVisible(type == Module::parameterType::booleanParam);
            comboOptions     .setVisible(type == Module::parameterType::choiceParam);
            browserButton    .setVisible(type == Module::parameterType::fileParam);
            selectedFileLabel.setVisible(type == Module::parameterType::fileParam);
        }
        
        juce::Label parameterTitleLabel;
        
        juce::Label         sliderLabel;
        StyledLinearSlider  slider;
        juce::Slider        integerStepper;
        OnOffButton         toggleButton;
        juce::ComboBox      comboOptions;
        juce::Label         selectedFileLabel;
        juce::TextButton    browserButton;
        std::unique_ptr<juce::FileChooser> fileChooser;
        
        MatrixMenuButton addModButton;
        juce::ValueTree data;
    };
    
    class ExpandableModuleHeader : public ExpandableListBox::ListItemHolder::Header, public juce::ValueTree::Listener
    {
    public:
        ExpandableModuleHeader(juce::String headerText)
        : ExpandableListBox::ListItemHolder::Header(headerText)
        {
            addAndMakeVisible(onOffButton);
            onOffButton.setClickingTogglesState(true);
        }
        
        ~ExpandableModuleHeader() override
        {
            if (data.isValid())
                data.removeListener(this);
        }
        
        void resized() override
        {
            ExpandableListBox::ListItemHolder::Header::resized();
            onOffButton.setBounds(getLocalBounds().removeFromLeft(int(Height * 1.4f)).reduced(6, 8));
        }
        
        void setData (juce::ValueTree _data)
        {
            //skip if new data invalid
            if (!_data.isValid())
                return;
            
            //remove listeners to old data
            if (data.isValid())
                data.removeListener (this);
            
            data = _data;
            data.addListener(this);
            
            onOffButton.setClickingTogglesState (true);
            onOffButton.setToggleState(data[Module::ParamIdents::ENABLED], juce::dontSendNotification);
            onOffButton.onStateChange = [this] ()
            {
                data.setProperty (Module::ParamIdents::ENABLED, onOffButton.getToggleState(), nullptr);
            };
            
            data.addListener (this);
        }
        
        void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override
        {
            if (tree == data && property == Module::ParamIdents::ENABLED)
                onOffButton.setToggleState(bool(data[Module::ParamIdents::ENABLED]), juce::dontSendNotification);
        }
        
    private:
        juce::ValueTree data;
        OnOffButton onOffButton;
    };
    
    class ExpandableModule : public ExpandableListItem
    {
        public:
        ExpandableModule(Context& _ctx)
        : ctx(_ctx)
        {
            
        }
        
        ~ExpandableModule() override
        {
            for (auto row : rows)
            {
                if (row)
                {
                    delete row;
                    row = nullptr;
                }
            }
        }
        
        void resized() override
        {
            auto area = getLocalBounds();
            for (auto row : rows)
            {
                row->setBounds(area.removeFromTop(ParameterRow::getRowHeight()));
            }
        }
        
        int getFullHeight() override
        {
            return rows.size() * ParameterRow::getRowHeight();
        }
        
        void setData(juce::ValueTree newData)
        {
            data = newData;
            
            for (auto row : data.getChildWithName(Module::ParamIdents::PARAMETERS))
            {
                auto paramRow = new ParameterRow(ctx);
                paramRow->setData(row);
                addAndMakeVisible(paramRow);
                rows.add(paramRow);
            }
            
            setSize(getWidth(), ParameterRow::getRowHeight() * rows.size());
        }
        
        private:
        
        juce::ValueTree getDataForRow (int rowNumber)
        {
            return data.getChildWithName(Module::ParamIdents::PARAMETERS).getChild(rowNumber);
        }
        
        juce::Array<juce::Component*> rows;
        juce::ValueTree data;
        sketchbook::Context& ctx;
    };
    
    public:
    
    ModuleGroupPage(Context& _ctx)
    : ctx(_ctx)
    {}
    
    void setData(juce::ValueTree newData)
    {
        data = newData;
        data.addListener(this);
        updateContent();
    }
    
    void updateContent()
    {
        auto tree = getModulesData(data);
        for (auto sourceData : tree)
        {
            auto header = std::make_unique<ExpandableModuleHeader>(sourceData[Module::ParamIdents::NAME].toString());
            auto panel = new ExpandableModule(ctx);
            panel->setData(sourceData);
            addItem(panel, "", std::move(header));
        }
        resized();
    }
    
    virtual juce::ValueTree getModulesData(juce::ValueTree fullData)=0;
    
    private:
    
    juce::ValueTree data;
    sketchbook::Context& ctx;
};

class Pages  : public juce::Component
{
    class ParametersPage : public ModuleGroupPage
    {
        public:
        
        ParametersPage(Context& _ctx)
        : ModuleGroupPage(_ctx)
        {}
        
        juce::ValueTree getModulesData(juce::ValueTree fullData)
        {
            return fullData.getChildWithName(Module::ParamIdents::MODULES);
        }
    };
    
    
    class ModulationSourcesPage : public ModuleGroupPage
    {
    public:
        
        ModulationSourcesPage(Context& _ctx)
        : ModuleGroupPage(_ctx)
        {}
        
        juce::ValueTree getModulesData(juce::ValueTree fullData) override
        {
            return fullData.getRoot().getChildWithName(Module::ParamIdents::MODULATION_SOURCES);
        }
    };
    
    class FXPage : public ModuleGroupPage
    {
    public:
        
        FXPage(Context& _ctx)
        : ModuleGroupPage(_ctx)
        {}
        
        juce::ValueTree getModulesData(juce::ValueTree fullData) override
        {
            return fullData.getRoot().getChildWithName(Module::ParamIdents::EFFECT_FILTERS);
        }
    };
    
    class ModulationsPage : public juce::ListBox, public juce::ListBoxModel, public juce::ValueTree::Listener
    {
        class ModulationRow : public juce::Component, public juce::ValueTree::Listener
        {
            public:
            ModulationRow(Context& ctx)
            : amount(ctx)
            {
                addAndMakeVisible (reverse);
                reverse.setButtonText ("Reverse");
                reverse.setClickingTogglesState (true);
                reverse.onClick = [this] ()
                {
                    data.setProperty (Module::ParamIdents::MOD_REVERSED, reverse.getToggleState(), nullptr);
                };
                
                addAndMakeVisible (biPolar);
                biPolar.setButtonText ("BiPolar");
                biPolar.setClickingTogglesState (true);
                biPolar.onClick = [this] ()
                {
                    data.setProperty (Module::ParamIdents::MOD_CENTRED, biPolar.getToggleState(), nullptr);
                };
                
                addAndMakeVisible (remove);
                
                addAndMakeVisible (amount);
                amount.setRange (-1.0f, 1.0f);
                amount.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                amount.onValueChange = [this] ()
                {
                    data.setProperty(Module::ParamIdents::MOD_AMOUNT, amount.getValue(), nullptr);
                };
                
                addAndMakeVisible(sourceLabel);
                
                addAndMakeVisible (remove);
                remove.setButtonText ("X");
                remove.onClick = [this] ()
                {
                    if (data.isValid())
                    {
                        auto parent = data.getParent();
                        
                        if (parent.isValid())
                            parent.removeChild(data, nullptr);
                    }
                };
            }
            
            ~ModulationRow() override
            {
                
            }
            
            void setData(juce::ValueTree _data)
            {
                if (data.isValid())
                    data.removeListener (this);
                
                data = _data;
                data.addListener(this);
                
                juce::String source        = data[Module::ParamIdents::MODULATION_SOURCE].toString();
                
                //TODO: traversing by get parents will be prone to brake if structure changes
                juce::String destModule = data.getParent().getParent().getParent()[Module::ParamIdents::NAME].toString();
                juce::String destParam  = data.getParent().getParent()[Module::ParamIdents::PARAMETER_NAME].toString();
                
                sourceLabel.setText(source + " -> " + destModule + ": " + destParam, juce::dontSendNotification);
                amount.setValue (data[Module::ParamIdents::MOD_AMOUNT], juce::dontSendNotification);
                reverse.setToggleState (data[Module::ParamIdents::MOD_REVERSED], juce::dontSendNotification);
                biPolar.setToggleState (data[Module::ParamIdents::MOD_CENTRED], juce::dontSendNotification);
            }
            
            void paint (juce::Graphics& g) override
            {
                
            }
            
            void resized() override
            {
                auto area = getLocalBounds().reduced(0, 2);
                
                sourceLabel.setBounds(area.removeFromLeft(area.getWidth() * 0.45));
                amount.setBounds(area.removeFromLeft(area.getHeight()).reduced(1.5));
                
                int width = area.getWidth() / 3;
                area.reduce(0, 3.5);
                remove .setBounds(area.removeFromRight(width).reduced(3.5));
                reverse.setBounds(area.removeFromRight(width).reduced(3.5));
                biPolar.setBounds(area.removeFromRight(width).reduced(3.5));
            }
            
            private:
            
            StyledSlider    amount;
            juce::TextButton      reverse;
            juce::TextButton      biPolar;
            juce::TextButton      remove;
            juce::ValueTree       data;
            juce::Label           sourceLabel;
            
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulationRow)
        };
        
        public:
        ModulationsPage(Context& _ctx)
        : ctx(_ctx)
        {
            setModel(this);
            setColour(juce::ListBox::ColourIds::backgroundColourId, Style::getInstance()->backgroundColour.withAlpha(0.f));
            setRowHeight(44);
        }
        
        ~ModulationsPage() override
        {
            
        }
        
        juce::Array<juce::ValueTree> recursiveSearch(juce::Identifier name, juce::ValueTree tree)
        {
            juce::Array<juce::ValueTree> output;
            for (auto child : tree)
            {
                if (child.hasType(name))
                    output.add(child);
                else
                    output.addArray(recursiveSearch(name, child));
            }
            return output;
        }
        
        void setData(juce::ValueTree newData)
        {
            data = newData;
            data.addListener(this);
            modulationDataList = recursiveSearch(Module::ParamIdents::MODULATION, data);
            updateContent();
        }
        
        void valueTreeChildAdded(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenAdded) override
        {
            if (childWhichHasBeenAdded.hasType(Module::ParamIdents::MODULATION))
            {
                modulationDataList = recursiveSearch(Module::ParamIdents::MODULATION, data);
                updateContent();
            }
        }
        
        void valueTreeChildRemoved(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override
        {
            if (childWhichHasBeenRemoved.hasType(Module::ParamIdents::MODULATION))
            {
                //rebuild the list instead of searching
                modulationDataList = recursiveSearch(Module::ParamIdents::MODULATION, data);
                updateContent();
            }
        }
        
        int getNumRows() override
        {
            return modulationDataList.size();
        }
        
        juce::Component* refreshComponentForRow (int rowNumber, bool isRowSelected, juce::Component *existingComponentToUpdate) override
        {
            auto data = getDataForRow(rowNumber);
            
            if (!data.isValid())
            {
                return nullptr;
            }
            
            if (existingComponentToUpdate != nullptr)
            {
                if (auto* row = dynamic_cast<ModulationRow*> (existingComponentToUpdate))
                {
                    row->setData (data);
                    return row;
                }
            }
            else
            {
                auto* row = new ModulationRow(ctx);
                row->setData(data);
                addAndMakeVisible (row);
                return row;
            }
            
            return nullptr;
        }
        
        void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override
        {
            return;
        }
        
        juce::ValueTree getDataForRow(int rowNumber)
        {
            if (modulationDataList.size() > rowNumber)
                return modulationDataList.getReference(rowNumber);
            
            //Trying to access a row that doesn't exist
            return juce::ValueTree();
        }
        
        private:
        
        Context& ctx;
        juce::ValueTree data;
        juce::Array<juce::ValueTree> modulationDataList;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulationsPage)
    };
    
    public:
    
    Pages(Context& _ctx)
    : parametersPage(_ctx)
    , modulationSourcesPage(_ctx)
    , modulationsPage(_ctx)
    , fxPage(_ctx)
    , ctx(_ctx)
    {
        pageList.add(&parametersPage);
        pageList.add(&fxPage);
        pageList.add(&modulationSourcesPage);
        pageList.add(&modulationsPage);
        
        addChildComponent(parametersPage);
        addChildComponent(fxPage);
        addChildComponent(modulationSourcesPage);
        addChildComponent(modulationsPage);

        //show first page on start
        showPage(0);
    }
    
    ~Pages() override
    {
        
    }
    
    void setData(juce::ValueTree newData)
    {
        parametersPage.setData(newData);
        modulationSourcesPage.setData(newData);
        modulationsPage.setData(newData);
        fxPage.setData(newData);
    }
    
    void resized() override
    {
        for (auto p : pageList)
        {
            p->setBounds(getLocalBounds());
        }
    }
    
    void showPage(int pageIndex)
    {
        if (pageIndex < pageList.size() && pageIndex >= 0)
        {
            pageList[selectedPage]->setVisible(false);
            pageList[pageIndex]->setVisible(true);
            selectedPage = pageIndex;
        }
        
        resized();
    }
    
    private:
    
    ParametersPage parametersPage;
    ModulationSourcesPage modulationSourcesPage;
    ModulationsPage modulationsPage;
    FXPage fxPage;
    juce::Array<juce::Component*> pageList;
    int selectedPage = 0;
    Context& ctx;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pages)
};

} // end namespace sketchbook
