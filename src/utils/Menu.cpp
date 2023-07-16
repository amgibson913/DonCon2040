#include "utils/Menu.h"

namespace Doncon::Utils {

const std::map<Menu::Page, const Menu::Descriptor> Menu::descriptors = {
    {Menu::Page::Main,                                                    //
     {Menu::Descriptor::Type::Root,                                       //
      "Settings",                                                         //
      {{"Mode", Menu::Descriptor::Action::GotoPageDeviceMode},            //
       {"Brightness", Menu::Descriptor::Action::GotoPageLedBrightness},   //
       {"Sensitvty", Menu::Descriptor::Action::GotoPageTriggerThreshold}, //
       {"Reset", Menu::Descriptor::Action::GotoPageReset},                //
       {"BOOTSEL", Menu::Descriptor::Action::GotoPageBootsel}},           //
      0,                                                                  //
      Menu::Page::None}},                                                 //

    {Menu::Page::DeviceMode,                                                 //
     {Menu::Descriptor::Type::Selection,                                     //
      "Mode",                                                                //
      {{"Swtch Tata", Menu::Descriptor::Action::ChangeUsbModeSwitchTatacon}, //
       {"Swtch Pro", Menu::Descriptor::Action::ChangeUsbModeSwitchHoripad},  //
       {"Dualshock3", Menu::Descriptor::Action::ChangeUsbModeDS3},           //
       {"PS4 Tata", Menu::Descriptor::Action::ChangeUsbModePS4Tatacon},      //
       {"Dualshock4", Menu::Descriptor::Action::ChangeUsbModeDS4},           //
       {"Xbox 360", Menu::Descriptor::Action::ChangeUsbModeXbox360},         //
       {"Debug", Menu::Descriptor::Action::ChangeUsbModeDebug}},             //
      0,                                                                     //
      Menu::Page::Main}},                                                    //

    {Menu::Page::TriggerThreshold,                                                   //
     {Menu::Descriptor::Type::Selection,                                             //
      "Sensitivity",                                                                 //
      {{"Ka Left", Menu::Descriptor::Action::GotoPageTriggerThresholdKaLeft},        //
       {"Don Left", Menu::Descriptor::Action::GotoPageTriggerThresholdDonLeft},      //
       {"Don Right", Menu::Descriptor::Action::GotoPageTriggerThresholdDonRight},    //
       {"Ka Right", Menu::Descriptor::Action::GotoPageTriggerThresholdKaRight},      //
       {"Scale Lvl", Menu::Descriptor::Action::GotoPageTriggerThresholdScaleLevel}}, //
      0,                                                                             //
      Menu::Page::Main}},                                                            //

    {Menu::Page::TriggerThresholdKaLeft,                           //
     {Menu::Descriptor::Type::Value,                               //
      "Trg Level Ka Left",                                         //
      {{"", Menu::Descriptor::Action::SetTriggerThresholdKaLeft}}, //
      4095,                                                        //
      Menu::Page::TriggerThreshold}},

    {Menu::Page::TriggerThresholdDonLeft,                           //
     {Menu::Descriptor::Type::Value,                                //
      "Trg Level Don Left",                                         //
      {{"", Menu::Descriptor::Action::SetTriggerThresholdDonLeft}}, //
      4095,                                                         //
      Menu::Page::TriggerThreshold}},

    {Menu::Page::TriggerThresholdDonRight,                           //
     {Menu::Descriptor::Type::Value,                                 //
      "Trg Level Don Right",                                         //
      {{"", Menu::Descriptor::Action::SetTriggerThresholdDonRight}}, //
      4095,                                                          //
      Menu::Page::TriggerThreshold}},

    {Menu::Page::TriggerThresholdKaRight,                           //
     {Menu::Descriptor::Type::Value,                                //
      "Trg Level Ka Right",                                         //
      {{"", Menu::Descriptor::Action::SetTriggerThresholdKaRight}}, //
      4095,                                                         //
      Menu::Page::TriggerThreshold}},

    {Menu::Page::TriggerThresholdScaleLevel,                           //
     {Menu::Descriptor::Type::Value,                                   //
      "Sensitivity Scale Lvl",                                         //
      {{"", Menu::Descriptor::Action::SetTriggerThresholdScaleLevel}}, //
      UINT8_MAX,                                                       //
      Menu::Page::TriggerThreshold}},

    {Menu::Page::LedBrightness,                           //
     {Menu::Descriptor::Type::Value,                      //
      "LED Brightness",                                   //
      {{"", Menu::Descriptor::Action::SetLedBrightness}}, //
      UINT8_MAX,                                          //
      Menu::Page::Main}},                                 //

    {Menu::Page::Reset,                              //
     {Menu::Descriptor::Type::Selection,             //
      "Reset all Settings?",                         //
      {{"No", Menu::Descriptor::Action::GotoParent}, //
       {"Yes", Menu::Descriptor::Action::DoReset}},  //
      0,                                             //
      Menu::Page::Main}},                            //

    {Menu::Page::Bootsel,                                         //
     {Menu::Descriptor::Type::Selection,                          //
      "Reboot to BOOTSEL",                                        //
      {{"Reboot?", Menu::Descriptor::Action::DoRebootToBootsel}}, //
      0,                                                          //
      Menu::Page::Main}},                                         //

    {Menu::Page::BootselMsg,                         //
     {Menu::Descriptor::Type::RebootInfo,            //
      "Ready to Flash...",                           //
      {{"BOOTSEL", Menu::Descriptor::Action::None}}, //
      0,                                             //
      Menu::Page::Main}},                            //
};

Menu::Menu(std::shared_ptr<SettingsStore> settings_store)
    : m_store(settings_store), m_active(false), m_state({Page::Main, 0}){};

void Menu::activate() {
    m_state = {Page::Main, 0};
    m_active = true;
}

static InputState::Controller checkPressed(const InputState::Controller &controller_state) {
    struct ButtonState {
        enum State {
            Idle,
            RepeatDelay,
            Repeat,
        };
        State state;
        uint32_t pressed_since;
        uint32_t last_repeat;
    };

    static const uint32_t repeat_delay = 1000;
    static const uint32_t repeat_interval = 20;

    static ButtonState state_north = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_east = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_south = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_west = {ButtonState::State::Idle, 0, 0};

    static ButtonState state_up = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_down = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_left = {ButtonState::State::Idle, 0, 0};
    static ButtonState state_right = {ButtonState::State::Idle, 0, 0};

    InputState::Controller result{{false, false, false, false},
                                  {false, false, false, false, false, false, false, false, false, false}};

    auto handle_button = [](ButtonState &button_state, bool input_state) {
        bool result = false;
        if (input_state) {
            uint32_t now = to_ms_since_boot(get_absolute_time());
            switch (button_state.state) {
            case ButtonState::State::Idle:
                result = true;
                button_state.state = ButtonState::State::RepeatDelay;
                button_state.pressed_since = now;
                break;
            case ButtonState::State::RepeatDelay:
                if ((now - button_state.pressed_since) > repeat_delay) {
                    result = true;
                    button_state.state = ButtonState::State::Repeat;
                    button_state.last_repeat = now;
                } else {
                    result = false;
                }
                break;
            case ButtonState::State::Repeat:
                if ((now - button_state.last_repeat) > repeat_interval) {
                    result = true;
                    button_state.last_repeat = now;
                } else {
                    result = false;
                }
                break;
            }
        } else {
            result = false;
            button_state.state = ButtonState::State::Idle;
        }

        return result;
    };

    result.buttons.north = handle_button(state_north, controller_state.buttons.north);
    result.buttons.east = handle_button(state_east, controller_state.buttons.east);
    result.buttons.south = handle_button(state_south, controller_state.buttons.south);
    result.buttons.west = handle_button(state_west, controller_state.buttons.west);

    result.dpad.up = handle_button(state_up, controller_state.dpad.up);
    result.dpad.down = handle_button(state_down, controller_state.dpad.down);
    result.dpad.left = handle_button(state_left, controller_state.dpad.left);
    result.dpad.right = handle_button(state_right, controller_state.dpad.right);

    return result;
}

uint16_t Menu::getCurrentSelection(Menu::Page page) {
    switch (page) {
    case Page::DeviceMode:
        return static_cast<uint16_t>(m_store->getUsbMode());
        break;
    case Page::TriggerThresholdKaLeft:
        return m_store->getTriggerThresholds().ka_left;
        break;
    case Page::TriggerThresholdDonLeft:
        return m_store->getTriggerThresholds().don_left;
        break;
    case Page::TriggerThresholdDonRight:
        return m_store->getTriggerThresholds().don_right;
        break;
    case Page::TriggerThresholdKaRight:
        return m_store->getTriggerThresholds().ka_right;
        break;
    case Page::TriggerThresholdScaleLevel:
        return m_store->getTriggerThresholdScaleLevel();
        break;
    case Page::LedBrightness:
        return m_store->getLedBrightness();
        break;
    case Page::Main:
    case Page::TriggerThreshold:
    case Page::Reset:
    case Page::Bootsel:
    case Page::BootselMsg:
    case Page::None:
        break;
    }

    return 0;
}

void Menu::gotoPage(Menu::Page page) {
    m_state.page = page;
    m_state.selection = getCurrentSelection(page);
}

void Menu::performSelectionAction(Menu::Descriptor::Action action) {
    auto descriptor_it = descriptors.find(m_state.page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    switch (action) {
    case Descriptor::Action::GotoPageDeviceMode:
        gotoPage(Page::DeviceMode);
        break;
    case Descriptor::Action::GotoPageTriggerThreshold:
        gotoPage(Page::TriggerThreshold);
        break;
    case Descriptor::Action::GotoPageTriggerThresholdKaLeft:
        gotoPage(Page::TriggerThresholdKaLeft);
        break;
    case Descriptor::Action::GotoPageTriggerThresholdDonLeft:
        gotoPage(Page::TriggerThresholdDonLeft);
        break;
    case Descriptor::Action::GotoPageTriggerThresholdDonRight:
        gotoPage(Page::TriggerThresholdDonRight);
        break;
    case Descriptor::Action::GotoPageTriggerThresholdKaRight:
        gotoPage(Page::TriggerThresholdKaRight);
        break;
    case Descriptor::Action::GotoPageTriggerThresholdScaleLevel:
        gotoPage(Page::TriggerThresholdScaleLevel);
        break;
    case Descriptor::Action::GotoPageLedBrightness:
        gotoPage(Page::LedBrightness);
        break;
    case Descriptor::Action::GotoPageReset:
        gotoPage(Page::Reset);
        break;
    case Descriptor::Action::GotoPageBootsel:
        gotoPage(Page::Bootsel);
        break;
    case Descriptor::Action::ChangeUsbModeSwitchTatacon:
        m_store->setUsbMode(USB_MODE_SWITCH_TATACON);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeUsbModeSwitchHoripad:
        m_store->setUsbMode(USB_MODE_SWITCH_HORIPAD);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeUsbModeDS3:
        m_store->setUsbMode(USB_MODE_DUALSHOCK3);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeUsbModePS4Tatacon:
        m_store->setUsbMode(USB_MODE_PS4_TATACON);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeUsbModeDS4:
        m_store->setUsbMode(USB_MODE_DUALSHOCK4);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeUsbModeXbox360:
        m_store->setUsbMode(USB_MODE_XBOX360);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::ChangeUsbModeDebug:
        m_store->setUsbMode(USB_MODE_DEBUG);
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::SetTriggerThresholdKaLeft:
    case Descriptor::Action::SetTriggerThresholdDonLeft:
    case Descriptor::Action::SetTriggerThresholdDonRight:
    case Descriptor::Action::SetTriggerThresholdKaRight:
    case Descriptor::Action::SetTriggerThresholdScaleLevel:
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::SetLedBrightness:
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::DoReset:
        m_store->reset();
        break;
    case Descriptor::Action::DoRebootToBootsel:
        m_store->scheduleReboot(true);
        gotoPage(Page::BootselMsg);
        break;
    case Descriptor::Action::GotoParent:
        gotoPage(descriptor_it->second.parent);
        break;
    case Descriptor::Action::None:
        break;
    }
}

void Menu::performValueAction(Menu::Descriptor::Action action, uint16_t value) {
    auto descriptor_it = descriptors.find(m_state.page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    switch (action) {
    case Descriptor::Action::SetTriggerThresholdKaLeft: {
        auto thresholds = m_store->getTriggerThresholds();
        thresholds.ka_left = value;
        m_store->setTriggerThresholds(thresholds);
    } break;
    case Descriptor::Action::SetTriggerThresholdDonLeft: {
        auto thresholds = m_store->getTriggerThresholds();
        thresholds.don_left = value;
        m_store->setTriggerThresholds(thresholds);
    } break;
    case Descriptor::Action::SetTriggerThresholdDonRight: {
        auto thresholds = m_store->getTriggerThresholds();
        thresholds.don_right = value;
        m_store->setTriggerThresholds(thresholds);
    } break;
    case Descriptor::Action::SetTriggerThresholdKaRight: {
        auto thresholds = m_store->getTriggerThresholds();
        thresholds.don_right = value;
        m_store->setTriggerThresholds(thresholds);
    } break;
    case Descriptor::Action::SetTriggerThresholdScaleLevel:
        m_store->setTriggerThresholdScaleLevel(value);
        break;
    case Descriptor::Action::SetLedBrightness:
        m_store->setLedBrightness(value);
        break;
    default:
        break;
    }
}

void Menu::update(const InputState::Controller &controller_state) {
    InputState::Controller pressed = checkPressed(controller_state);

    auto descriptor_it = descriptors.find(m_state.page);
    if (descriptor_it == descriptors.end()) {
        assert(false);
        return;
    }

    if (descriptor_it->second.type == Descriptor::Type::RebootInfo) {
        m_active = false;
    } else if (pressed.dpad.left) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
            if (m_state.selection == 0) {
                m_state.selection = descriptor_it->second.items.size() - 1;
            } else {
                m_state.selection--;
            }
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.dpad.right) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
            if (m_state.selection == descriptor_it->second.items.size() - 1) {
                m_state.selection = 0;
            } else {
                m_state.selection++;
            }
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.dpad.up) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            if (m_state.selection < descriptor_it->second.max_value) {
                m_state.selection++;
                performValueAction(descriptor_it->second.items.at(0).second, m_state.selection);
            }
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.dpad.down) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            if (m_state.selection > 0) {
                m_state.selection--;
                performValueAction(descriptor_it->second.items.at(0).second, m_state.selection);
            }
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.buttons.south) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
        case Descriptor::Type::Selection:
            gotoPage(descriptor_it->second.parent);
            break;
        case Descriptor::Type::Root:
            m_active = false;
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    } else if (pressed.buttons.east) {
        switch (descriptor_it->second.type) {
        case Descriptor::Type::Value:
            performSelectionAction(descriptor_it->second.items.at(0).second);
            break;
        case Descriptor::Type::Selection:
        case Descriptor::Type::Root:
            performSelectionAction(descriptor_it->second.items.at(m_state.selection).second);
            break;
        case Descriptor::Type::RebootInfo:
            break;
        }
    }
}

bool Menu::active() { return m_active; }

Menu::State Menu::getState() { return m_state; }

} // namespace Doncon::Utils
