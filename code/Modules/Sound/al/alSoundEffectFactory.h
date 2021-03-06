#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::_priv::alSoundEffectFactory
    @ingroup _priv
    @brief OpenAL implementation of sound effect factory
*/
#include "Sound/Core/soundEffectFactoryBase.h"

namespace Oryol {
namespace _priv {

class alSoundEffectFactory : public soundEffectFactoryBase {
public:
    /// setup resource
    ResourceState::Code setupResource(soundEffect& effect);
    /// setup with 'raw' data
    ResourceState::Code setupResource(soundEffect& effect, const void* data, int32 size);
    /// destroy a resource
    void destroyResource(soundEffect& effect);

private:
    /// create the alBuffer and alSources for soundEffect
    void createBufferAndSources(soundEffect& effect, const int16* samples, int32 numSamples);

    static const int32 MaxNumBufferSamples = 512 * 1024;
    int16 sampleBuffer[MaxNumBufferSamples];
};

} // namespace _priv
} // namespace Oryol