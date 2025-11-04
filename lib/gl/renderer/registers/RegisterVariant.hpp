#ifndef _REGISTER_VARIANT_HPP_
#define _REGISTER_VARIANT_HPP_

#include "renderer/registers/ColorBufferAddrReg.hpp"
#include "renderer/registers/ColorBufferClearColorReg.hpp"
#include "renderer/registers/DepthBufferAddrReg.hpp"
#include "renderer/registers/DepthBufferClearDepthReg.hpp"
#include "renderer/registers/FeatureEnableReg.hpp"
#include "renderer/registers/FogColorReg.hpp"
#include "renderer/registers/FragmentPipelineReg.hpp"
#include "renderer/registers/RenderResolutionReg.hpp"
#include "renderer/registers/ScissorEndReg.hpp"
#include "renderer/registers/ScissorStartReg.hpp"
#include "renderer/registers/StencilBufferAddrReg.hpp"
#include "renderer/registers/StencilReg.hpp"
#include "renderer/registers/TexEnvColorReg.hpp"
#include "renderer/registers/TexEnvReg.hpp"
#include "renderer/registers/TmuTextureReg.hpp"
#include "renderer/registers/YOffsetReg.hpp"

#include <variant>
namespace rr
{
using RegisterVariant = std::variant<
    std::monostate,
    ColorBufferAddrReg,
    ColorBufferClearColorReg,
    DepthBufferAddrReg,
    DepthBufferClearDepthReg,
    FeatureEnableReg,
    FogColorReg,
    FragmentPipelineReg,
    RenderResolutionReg,
    ScissorEndReg,
    ScissorStartReg,
    StencilBufferAddrReg,
    StencilReg,
    TexEnvColorReg,
    TexEnvReg,
    TmuTextureReg,
    YOffsetReg>;

} // namespace rr

#endif // _REGISTER_VARIANT_HPP_