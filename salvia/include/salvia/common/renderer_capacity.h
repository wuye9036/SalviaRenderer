#pragma once

#include <eflib/platform/typedefs.h>

namespace salvia {

constexpr uint32_t MAX_VS_INPUT_ATTRS = 8;
constexpr uint32_t MAX_VS_OUTPUT_ATTRS = 7;
constexpr uint32_t MAX_RENDER_TARGETS = 8;
constexpr uint32_t MAX_INPUT_SLOTS = 32;
constexpr uint32_t MAX_COMMAND_QUEUE = 32;
constexpr uint32_t MAX_RENDER_TARGET_WIDTH = 8192;
constexpr uint32_t MAX_RENDER_TARGET_HEIGHT = 8192;
constexpr uint32_t MAX_SAMPLE_COUNT = 16;
constexpr uint32_t SAMPLE_MASK = 0xFFFF;

} // namespace salvia
