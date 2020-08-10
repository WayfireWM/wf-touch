#include "math.hpp"

uint32_t get_move_direction(const finger_t& finger)
{
    point_t diff = finger.current - finger.origin;

    uint32_t result = 0;
    if (diff.x < 0)
    {
        result |= MOVE_DIRECTION_LEFT;
    } else if (diff.x > 0)
    {
        result |= MOVE_DIRECTION_RIGHT;
    }

    if (diff.y < 0)
    {
        result |= MOVE_DIRECTION_UP;
    } else if (diff.y > 0)
    {
        result |= MOVE_DIRECTION_DOWN;
    }

    return result;
}
