#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::RasterizerState
    @ingroup Gfx
    @brief rasterizer state flags
*/
#include "Core/Types.h"
#include "Gfx/Core/Enums.h"

namespace Oryol {
    
class RasterizerState {
public:
    union {
        #pragma pack(push,1)
        struct {
            uint16 CullFaceEnabled : 1;
            uint16 DepthOffsetEnabled : 1;
            uint16 ScissorTestEnabled : 1;
            uint16 DitherEnabled : 1;
            uint16 AlphaToCoverageEnabled : 1;
            Face::Code CullFace : 3;
            uint16 SampleCount : 4;
        };
        #pragma pack(pop)
        uint16 Hash;
    };

    /// constructor
    RasterizerState() {
        static_assert(sizeof(RasterizerState) == 2, "sizeof(RasterizerState) is not 4, bitfield packing problem?");
        this->Hash = 0;
        this->CullFaceEnabled = false;
        this->DepthOffsetEnabled = false;
        this->ScissorTestEnabled = false;
        this->DitherEnabled = true;
        this->AlphaToCoverageEnabled = false;
        this->CullFace = Face::Back;
        this->SampleCount = 1;
    }
    /// equality
    bool operator==(const RasterizerState& rhs) const {
        return this->Hash == rhs.Hash;
    };
    /// inequality
    bool operator!=(const RasterizerState& rhs) const {
        return this->Hash != rhs.Hash;
    };
};

} // namespace Oryol
