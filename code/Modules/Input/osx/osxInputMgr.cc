//------------------------------------------------------------------------------
//  osxInputMgr.cc
//  osxInputMgr.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "osxInputMgr.h"
#include "Core/Core.h"
#include "Core/RunLoop.h"
#include "Core/osx/osxBridge.h"
#include "Input/InputProtocol.h"

namespace Oryol {
namespace _priv {

osxInputMgr* osxInputMgr::self = nullptr;

static Key::Code keyTable[ORYOL_OSXBRIDGE_KEY_LAST + 1];

//------------------------------------------------------------------------------
osxInputMgr::osxInputMgr() :
runLoopId(RunLoop::InvalidId) {
    o_assert(nullptr == self);
    self = this;
}

//------------------------------------------------------------------------------
osxInputMgr::~osxInputMgr() {
    o_assert(nullptr != self);
    self = nullptr;
}

//------------------------------------------------------------------------------
void
osxInputMgr::setup(const InputSetup& setup) {
    
    inputMgrBase::setup(setup);
    this->keyboard.Attached = true;
    this->mouse.Attached = true;
    
    this->setupKeyTable();
    this->setupCallbacks();
    this->setCursorMode(CursorMode::Normal);
    
    // attach our reset callback to the global runloop
    this->runLoopId = Core::PostRunLoop()->Add([this]() { this->reset(); });    
}

//------------------------------------------------------------------------------
void
osxInputMgr::discard() {
    
    this->discardCallbacks();
    Core::PostRunLoop()->Remove(this->runLoopId);
    this->runLoopId = RunLoop::InvalidId;
    
    inputMgrBase::discard();
}

//------------------------------------------------------------------------------
void
osxInputMgr::setupCallbacks() {
    osxBridge* bridge = osxBridge::ptr();
    bridge->callbacks.key = keyCallback;
    bridge->callbacks.character = charCallback;
    bridge->callbacks.mouseButton = mouseButtonCallback;
    bridge->callbacks.cursorPos = cursorPosCallback;
    bridge->callbacks.cursorEnter = cursorEnterCallback;
    bridge->callbacks.scroll = scrollCallback;
}

//------------------------------------------------------------------------------
void
osxInputMgr::discardCallbacks() {
    osxBridge* bridge = osxBridge::ptr();
    bridge->callbacks.key = nullptr;
    bridge->callbacks.character = nullptr;
    bridge->callbacks.mouseButton = nullptr;
    bridge->callbacks.cursorPos = nullptr;
    bridge->callbacks.cursorEnter = nullptr;
    bridge->callbacks.scroll = nullptr;
}

//------------------------------------------------------------------------------
void
osxInputMgr::keyCallback(int osxKey, int /*scancode*/, int action, int /*mods*/) {
    if (nullptr != self) {
        Key::Code key = self->mapKey(osxKey);
        if (Key::InvalidKey != key) {
            auto msg = InputProtocol::Key::Create();
            msg->SetKey(key);
            if (action == ORYOL_OSXBRIDGE_PRESS) {
                self->keyboard.onKeyDown(key);
                msg->SetDown(true);
            }
            else if (action == ORYOL_OSXBRIDGE_RELEASE) {
                self->keyboard.onKeyUp(key);
                msg->SetUp(true);
            }
            else {
                self->keyboard.onKeyRepeat(key);
                msg->SetRepeat(true);
            }
            self->notifyHandlers(msg);
        }
    }
}

//------------------------------------------------------------------------------
void
osxInputMgr::charCallback(unsigned int unicode) {
    if (nullptr != self) {
        self->keyboard.onChar((wchar_t)unicode);
        auto msg = InputProtocol::WChar::Create();
        msg->SetWChar((wchar_t) unicode);
        self->notifyHandlers(msg);
    }
}

//------------------------------------------------------------------------------
void
osxInputMgr::mouseButtonCallback(int button, int action, int mods) {
    if (nullptr != self) {
        Mouse::Button btn;
        switch (button) {
            case ORYOL_OSXBRIDGE_MOUSE_BUTTON_LEFT:     btn = Mouse::LMB; break;
            case ORYOL_OSXBRIDGE_MOUSE_BUTTON_RIGHT:    btn = Mouse::RMB; break;
            case ORYOL_OSXBRIDGE_MOUSE_BUTTON_MIDDLE:   btn = Mouse::MMB; break;
            default:                                    btn = Mouse::InvalidButton; break;
        }
        if (btn != Mouse::InvalidButton) {
            auto msg = InputProtocol::MouseButton::Create();
            msg->SetMouseButton(btn);
            if (action == ORYOL_OSXBRIDGE_PRESS) {
                self->mouse.onButtonDown(btn);
                msg->SetDown(true);
            }
            else if (action == ORYOL_OSXBRIDGE_RELEASE) {
                self->mouse.onButtonUp(btn);
                msg->SetUp(true);
            }
            self->notifyHandlers(msg);
        }
    }
}

//------------------------------------------------------------------------------
void
osxInputMgr::cursorPosCallback(float64 x, float64 y) {
    if (nullptr != self) {
        const glm::vec2 pos((float32)x, (float32)y);
        self->mouse.onPosMov(pos);
        auto msg = InputProtocol::MouseMove::Create();
        msg->SetMovement(self->mouse.Movement);
        msg->SetPosition(self->mouse.Position);
        self->notifyHandlers(msg);
    }
}

//------------------------------------------------------------------------------
void
osxInputMgr::scrollCallback(float64 xOffset, float64 yOffset) {
    if (nullptr != self) {
        const glm::vec2 scroll((float32)xOffset, (float32)yOffset);
        self->mouse.Scroll = scroll;
        auto msg = InputProtocol::MouseScroll::Create();
        msg->SetScroll(scroll);
        self->notifyHandlers(msg);
    }
}

//------------------------------------------------------------------------------
void
osxInputMgr::cursorEnterCallback(bool entered) {
    Log::Info("cursorenter: %d\n", entered);
}

//------------------------------------------------------------------------------
Key::Code
osxInputMgr::mapKey(int osxBridgeKey) const {
    if ((osxBridgeKey >= 0) && (osxBridgeKey <= ORYOL_OSXBRIDGE_KEY_LAST)) {
        return keyTable[osxBridgeKey];
    }
    else {
        return Key::InvalidKey;
    }
}

//------------------------------------------------------------------------------
void
osxInputMgr::setupKeyTable() {

    for (int32 i = 0; i <= ORYOL_OSXBRIDGE_KEY_LAST; i++) {
        keyTable[i] = Key::InvalidKey;
    }
    
    keyTable[ORYOL_OSXBRIDGE_KEY_SPACE]          = Key::Space;
    keyTable[ORYOL_OSXBRIDGE_KEY_APOSTROPHE]     = Key::Apostrophe;
    keyTable[ORYOL_OSXBRIDGE_KEY_COMMA]          = Key::Comma;
    keyTable[ORYOL_OSXBRIDGE_KEY_MINUS]          = Key::Minus;
    keyTable[ORYOL_OSXBRIDGE_KEY_PERIOD]         = Key::Period;
    keyTable[ORYOL_OSXBRIDGE_KEY_SLASH]          = Key::Slash;
    keyTable[ORYOL_OSXBRIDGE_KEY_0]              = Key::N0;
    keyTable[ORYOL_OSXBRIDGE_KEY_1]              = Key::N1;
    keyTable[ORYOL_OSXBRIDGE_KEY_2]              = Key::N2;
    keyTable[ORYOL_OSXBRIDGE_KEY_3]              = Key::N3;
    keyTable[ORYOL_OSXBRIDGE_KEY_4]              = Key::N4;
    keyTable[ORYOL_OSXBRIDGE_KEY_5]              = Key::N5;
    keyTable[ORYOL_OSXBRIDGE_KEY_6]              = Key::N6;
    keyTable[ORYOL_OSXBRIDGE_KEY_7]              = Key::N7;
    keyTable[ORYOL_OSXBRIDGE_KEY_8]              = Key::N8;
    keyTable[ORYOL_OSXBRIDGE_KEY_9]              = Key::N9;
    keyTable[ORYOL_OSXBRIDGE_KEY_SEMICOLON]      = Key::Semicolon;
    keyTable[ORYOL_OSXBRIDGE_KEY_EQUAL]          = Key::Equal;
    keyTable[ORYOL_OSXBRIDGE_KEY_A]              = Key::A;
    keyTable[ORYOL_OSXBRIDGE_KEY_B]              = Key::B;
    keyTable[ORYOL_OSXBRIDGE_KEY_C]              = Key::C;
    keyTable[ORYOL_OSXBRIDGE_KEY_D]              = Key::D;
    keyTable[ORYOL_OSXBRIDGE_KEY_E]              = Key::E;
    keyTable[ORYOL_OSXBRIDGE_KEY_F]              = Key::F;
    keyTable[ORYOL_OSXBRIDGE_KEY_G]              = Key::G;
    keyTable[ORYOL_OSXBRIDGE_KEY_H]              = Key::H;
    keyTable[ORYOL_OSXBRIDGE_KEY_I]              = Key::I;
    keyTable[ORYOL_OSXBRIDGE_KEY_J]              = Key::J;
    keyTable[ORYOL_OSXBRIDGE_KEY_K]              = Key::K;
    keyTable[ORYOL_OSXBRIDGE_KEY_L]              = Key::L;
    keyTable[ORYOL_OSXBRIDGE_KEY_M]              = Key::M;
    keyTable[ORYOL_OSXBRIDGE_KEY_N]              = Key::N;
    keyTable[ORYOL_OSXBRIDGE_KEY_O]              = Key::O;
    keyTable[ORYOL_OSXBRIDGE_KEY_P]              = Key::P;
    keyTable[ORYOL_OSXBRIDGE_KEY_Q]              = Key::Q;
    keyTable[ORYOL_OSXBRIDGE_KEY_R]              = Key::R;
    keyTable[ORYOL_OSXBRIDGE_KEY_S]              = Key::S;
    keyTable[ORYOL_OSXBRIDGE_KEY_T]              = Key::T;
    keyTable[ORYOL_OSXBRIDGE_KEY_U]              = Key::U;
    keyTable[ORYOL_OSXBRIDGE_KEY_V]              = Key::V;
    keyTable[ORYOL_OSXBRIDGE_KEY_W]              = Key::W;
    keyTable[ORYOL_OSXBRIDGE_KEY_X]              = Key::X;
    keyTable[ORYOL_OSXBRIDGE_KEY_Y]              = Key::Y;
    keyTable[ORYOL_OSXBRIDGE_KEY_Z]              = Key::Z;
    keyTable[ORYOL_OSXBRIDGE_KEY_LEFT_BRACKET]   = Key::LeftBracket;
    keyTable[ORYOL_OSXBRIDGE_KEY_BACKSLASH]      = Key::BackSlash;
    keyTable[ORYOL_OSXBRIDGE_KEY_RIGHT_BRACKET]  = Key::RightBracket;
    keyTable[ORYOL_OSXBRIDGE_KEY_GRAVE_ACCENT]   = Key::GraveAccent;
    keyTable[ORYOL_OSXBRIDGE_KEY_WORLD_1]        = Key::World1;
    keyTable[ORYOL_OSXBRIDGE_KEY_WORLD_2]        = Key::World2;
    keyTable[ORYOL_OSXBRIDGE_KEY_ESCAPE]         = Key::Escape;
    keyTable[ORYOL_OSXBRIDGE_KEY_ENTER]          = Key::Enter;
    keyTable[ORYOL_OSXBRIDGE_KEY_TAB]            = Key::Tab;
    keyTable[ORYOL_OSXBRIDGE_KEY_BACKSPACE]      = Key::BackSpace;
    keyTable[ORYOL_OSXBRIDGE_KEY_INSERT]         = Key::Insert;
    keyTable[ORYOL_OSXBRIDGE_KEY_DELETE]         = Key::Delete;
    keyTable[ORYOL_OSXBRIDGE_KEY_RIGHT]          = Key::Right;
    keyTable[ORYOL_OSXBRIDGE_KEY_LEFT]           = Key::Left;
    keyTable[ORYOL_OSXBRIDGE_KEY_DOWN]           = Key::Down;
    keyTable[ORYOL_OSXBRIDGE_KEY_UP]             = Key::Up;
    keyTable[ORYOL_OSXBRIDGE_KEY_PAGE_UP]        = Key::PageUp;
    keyTable[ORYOL_OSXBRIDGE_KEY_PAGE_DOWN]      = Key::PageDown;
    keyTable[ORYOL_OSXBRIDGE_KEY_HOME]           = Key::Home;
    keyTable[ORYOL_OSXBRIDGE_KEY_END]            = Key::End;
    keyTable[ORYOL_OSXBRIDGE_KEY_CAPS_LOCK]      = Key::CapsLock;
    keyTable[ORYOL_OSXBRIDGE_KEY_SCROLL_LOCK]    = Key::ScrollLock;
    keyTable[ORYOL_OSXBRIDGE_KEY_NUM_LOCK]       = Key::NumLock;
    keyTable[ORYOL_OSXBRIDGE_KEY_PRINT_SCREEN]   = Key::PrintScreen;
    keyTable[ORYOL_OSXBRIDGE_KEY_PAUSE]          = Key::Pause;
    keyTable[ORYOL_OSXBRIDGE_KEY_F1]             = Key::F1;
    keyTable[ORYOL_OSXBRIDGE_KEY_F2]             = Key::F2;
    keyTable[ORYOL_OSXBRIDGE_KEY_F3]             = Key::F3;
    keyTable[ORYOL_OSXBRIDGE_KEY_F4]             = Key::F4;
    keyTable[ORYOL_OSXBRIDGE_KEY_F5]             = Key::F5;
    keyTable[ORYOL_OSXBRIDGE_KEY_F6]             = Key::F6;
    keyTable[ORYOL_OSXBRIDGE_KEY_F7]             = Key::F7;
    keyTable[ORYOL_OSXBRIDGE_KEY_F8]             = Key::F8;
    keyTable[ORYOL_OSXBRIDGE_KEY_F9]             = Key::F9;
    keyTable[ORYOL_OSXBRIDGE_KEY_F10]            = Key::F10;
    keyTable[ORYOL_OSXBRIDGE_KEY_F11]            = Key::F11;
    keyTable[ORYOL_OSXBRIDGE_KEY_F12]            = Key::F12;
    keyTable[ORYOL_OSXBRIDGE_KEY_F13]            = Key::F13;
    keyTable[ORYOL_OSXBRIDGE_KEY_F14]            = Key::F14;
    keyTable[ORYOL_OSXBRIDGE_KEY_F15]            = Key::F15;
    keyTable[ORYOL_OSXBRIDGE_KEY_F16]            = Key::F16;
    keyTable[ORYOL_OSXBRIDGE_KEY_F17]            = Key::F17;
    keyTable[ORYOL_OSXBRIDGE_KEY_F18]            = Key::F18;
    keyTable[ORYOL_OSXBRIDGE_KEY_F19]            = Key::F19;
    keyTable[ORYOL_OSXBRIDGE_KEY_F20]            = Key::F20;
    keyTable[ORYOL_OSXBRIDGE_KEY_F21]            = Key::F21;
    keyTable[ORYOL_OSXBRIDGE_KEY_F22]            = Key::F22;
    keyTable[ORYOL_OSXBRIDGE_KEY_F23]            = Key::F23;
    keyTable[ORYOL_OSXBRIDGE_KEY_F24]            = Key::F24;
    keyTable[ORYOL_OSXBRIDGE_KEY_F25]            = Key::F25;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_0]           = Key::Num0;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_1]           = Key::Num1;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_2]           = Key::Num2;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_3]           = Key::Num3;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_4]           = Key::Num4;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_5]           = Key::Num5;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_6]           = Key::Num6;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_7]           = Key::Num7;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_8]           = Key::Num8;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_9]           = Key::Num9;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_DECIMAL]     = Key::NumDecimal;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_DIVIDE]      = Key::NumDivide;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_MULTIPLY]    = Key::NumMultiply;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_SUBTRACT]    = Key::NumSubtract;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_ADD]         = Key::NumAdd;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_ENTER]       = Key::NumEnter;
    keyTable[ORYOL_OSXBRIDGE_KEY_KP_EQUAL]       = Key::NumEqual;
    keyTable[ORYOL_OSXBRIDGE_KEY_LEFT_SHIFT]     = Key::LeftShift;
    keyTable[ORYOL_OSXBRIDGE_KEY_LEFT_CONTROL]   = Key::LeftControl;
    keyTable[ORYOL_OSXBRIDGE_KEY_LEFT_ALT]       = Key::LeftAlt;
    keyTable[ORYOL_OSXBRIDGE_KEY_LEFT_SUPER]     = Key::LeftSuper;
    keyTable[ORYOL_OSXBRIDGE_KEY_RIGHT_SHIFT]    = Key::RightShift;
    keyTable[ORYOL_OSXBRIDGE_KEY_RIGHT_CONTROL]  = Key::RightControl;
    keyTable[ORYOL_OSXBRIDGE_KEY_RIGHT_ALT]      = Key::RightAlt;
    keyTable[ORYOL_OSXBRIDGE_KEY_RIGHT_SUPER]    = Key::RightSuper;
    keyTable[ORYOL_OSXBRIDGE_KEY_MENU]           = Key::Menu;
}

} // namespace _priv
} // namespace Oryol


