#pragma once
/* Private header */

#include <wayfire/touch/touch.hpp>
#include <cstdint>

using namespace wf::touch;

/** Find direction of movement, a bitmask of move_direction_t */
uint32_t get_move_direction(const finger_t& finger);

/** Find length of perpendicular direction drag */
double get_incorrect_drag_distance(const finger_t& finger,
    uint32_t direction);

/** Get the pinch scale */
double get_pinch_scale(const gesture_state_t& state);

/** Get the rotation angle, works for rotation < 180 degrees. */
double get_rotation_angle(const gesture_state_t& state);
