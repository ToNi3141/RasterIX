#ifndef COMMAND_VARIANT_HPP_
#define COMMAND_VARIANT_HPP_

#include "renderer/commands/FogLutStreamCmd.hpp"
#include "renderer/commands/FramebufferCmd.hpp"
#include "renderer/commands/NopCmd.hpp"
#include "renderer/commands/PushVertexCmd.hpp"
#include "renderer/commands/SetVertexCtxCmd.hpp"
#include "renderer/commands/TextureStreamCmd.hpp"
#include "renderer/commands/TriangleStreamCmd.hpp"
#include "renderer/commands/WriteRegisterCmd.hpp"

#include "renderer/registers/BaseColorReg.hpp"

#include <variant>

namespace rr
{
using CommandVariant = std::variant<
    FramebufferCmd,
    FogLutStreamCmd,
    NopCmd,
    PushVertexCmd,
    SetVertexCtxCmd,
    TextureStreamCmd,
    TriangleStreamCmd,
    WriteRegisterCmd>;
} // namespace rr

#endif // COMMAND_VARIANT_HPP_