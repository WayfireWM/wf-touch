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

const uint32_t lu = MOVE_DIRECTION_LEFT | MOVE_DIRECTION_UP;
const uint32_t ld = MOVE_DIRECTION_LEFT | MOVE_DIRECTION_DOWN;
const uint32_t rd = MOVE_DIRECTION_RIGHT | MOVE_DIRECTION_DOWN;
const uint32_t ru = MOVE_DIRECTION_RIGHT | MOVE_DIRECTION_UP;

TEST_CASE("get_move_in_direction")
{
    CHECK(get_move_direction(finger_in_dir(1, 0)) == MOVE_DIRECTION_RIGHT);
    CHECK(get_move_direction(finger_in_dir(-1, -1)) == lu);
    CHECK(get_move_direction(finger_in_dir(1, 1)) == rd);
    CHECK(get_move_direction(finger_in_dir(0, 0)) == 0);
}

TEST_CASE("get_incorrect_drag_distance")
{
    CHECK(get_incorrect_drag_distance(finger_in_dir(-1, -1), lu) ==
        doctest::Approx(0));
    CHECK(get_incorrect_drag_distance(finger_in_dir(-1, -1), ru) ==
        doctest::Approx(std::sqrt(2)));
    CHECK(get_incorrect_drag_distance(finger_in_dir(-1, -1), ld) ==
        doctest::Approx(std::sqrt(2)));
    CHECK(get_incorrect_drag_distance(finger_in_dir(5, 5), MOVE_DIRECTION_LEFT)
        == doctest::Approx(5));
}

TEST_CASE("finger_t")
{
    CHECK(finger_in_dir(1, 1).delta() == point_t{1, 1});
}

static void compare_point(const point_t& a, const point_t& b)
{
    CHECK(a.x == doctest::Approx(b.x));
    CHECK(a.y == doctest::Approx(b.y));
}

static void compare_finger(const finger_t& a, const finger_t& b)
{
    compare_point(a.origin, b.origin);
    compare_point(a.current, b.current);
}

TEST_CASE("gesture_state_t")
{
    gesture_state_t state;
    state.fingers[0] = finger_in_dir(1, 2);
    state.fingers[1] = finger_in_dir(3, 4);
    state.fingers[2] = finger_in_dir(5, 6);
    compare_finger(state.get_center(), finger_in_dir(3, 4));
}

