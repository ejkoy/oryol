#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::_priv::tapDetector
    @ingroup _priv
    @brief singletap- and multitap-gesture recognizer
*/
#include "Input/touch/touchEvent.h"
#include "Input/touch/gestureState.h"
#include "glm/vec2.hpp"

namespace Oryol {
namespace _priv {
    
class tapDetector {
public:
    /// timeout for touch in milliseconds
    static const int32 TouchTimeout = 250;
    /// timeout between multiple taps
    static const int32 TapTimeout = 750;
    /// max pixel distance from tap start
    static const int32 MaxTouchDistance = 20;
    /// max pixel distance between taps
    static const int32 MaxTapDistance = 30;
    
    /// number of required taps
    int32 numRequiredTaps = 1;
    /// feed new touch event and return detected state (if any)
    gestureState::Code detect(const touchEvent& newEvent);
    /// reset the detector
    void reset();
    /// get current tap position
    glm::vec2 position;
    
private:
    /// check if 2 touch events are within the tap max distance
    bool withinDistance(const touchEvent& newEvent, const touchEvent& oldEvent, int32 maxDist) const;
    /// check if 2 touch events are within a timeout
    bool withinTimeout(const touchEvent& newEvent, const touchEvent& oldEvent, int32 timeOutMs) const;
    
    touchEvent tapEvent;
    int32 tapCount = 0;
};
    
} // namespace _priv
} // namespace Oryol