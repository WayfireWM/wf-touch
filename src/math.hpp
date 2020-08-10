#pragma once
/* Private header */

#include <wayfire/touch/touch.hpp>
#include <cstdint>

using namespace wf::touch;

/** Find direction of movement, a bitmask of move_direction_t */
uint32_t get_move_direction(const finger_t& finger);
