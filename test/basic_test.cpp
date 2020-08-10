#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "src/math.hpp"

static finger_t finger_in_dir(double x, double y)
{
    return finger_t {
        .origin = {0, 0},
            .current = {x, y}
    };
}

TEST_CASE("get_move_in_direction")
{
    const uint32_t lu = MOVE_DIRECTION_LEFT | MOVE_DIRECTION_UP;
    const uint32_t rd = MOVE_DIRECTION_RIGHT | MOVE_DIRECTION_DOWN;

    CHECK(get_move_direction(finger_in_dir(1, 0)) == MOVE_DIRECTION_RIGHT);
    CHECK(get_move_direction(finger_in_dir(-1, -1)) == lu);
    CHECK(get_move_direction(finger_in_dir(1, 1)) == rd);
    CHECK(get_move_direction(finger_in_dir(0, 0)) == 0);
}

TEST_CASE("finger_t")
{
    CHECK(finger_in_dir(1, 1).delta() == point_t{1, 1});
}
