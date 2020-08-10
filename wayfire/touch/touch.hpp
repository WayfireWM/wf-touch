#pragma once

#include <glm/vec2.hpp>
#include <map>

namespace wf
{
namespace touch
{
using point_t = glm::dvec2;

struct finger_t
{
    point_t origin;
    point_t current;

    /** Get movement vector */
    point_t delta() const;
};

/**
 * Movement direction.
 */
enum move_direction_t
{
    MOVE_DIRECTION_LEFT  = (1 << 0),
    MOVE_DIRECTION_RIGHT = (1 << 1),
    MOVE_DIRECTION_UP    = (1 << 2),
    MOVE_DIRECTION_DOWN  = (1 << 3),
};

/**
 * Contains all fingers.
 */
struct gesture_state_t
{
  public:
    // finger_id -> finger_t
    std::map<int, finger_t> fingers;

    /**
     * Find the center of the fingers.
     */
    finger_t get_center() const;

};


}
}
