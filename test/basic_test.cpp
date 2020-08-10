#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "shared.hpp"

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

TEST_CASE("get_pinch_scale")
{
    gesture_state_t state;
    state.fingers[0] = finger_2p(1, 0, 2, 1);
    state.fingers[1] = finger_2p(-1, -2, -3, -4);
    CHECK(get_pinch_scale(state) > 2);

    std::swap(state.fingers[0].origin, state.fingers[0].current);
    std::swap(state.fingers[1].origin, state.fingers[1].current);
    CHECK(get_pinch_scale(state) < 0.5);

    state.fingers[0] = finger_2p(1, 1, 1, 1);
    state.fingers[1] = finger_2p(2, 2, 2, 2);
    CHECK(get_pinch_scale(state) == doctest::Approx(1));
}

TEST_CASE("get_rotation_angle")
{
    gesture_state_t state;
    state.fingers[0] = finger_2p(0, 1, 1, 0);
    state.fingers[1] = finger_2p(1, 0, 0, -1);
    state.fingers[2] = finger_2p(0, -1, -1, 0);
    state.fingers[3] = finger_2p(-1, 0, 0, 1);
    CHECK(get_rotation_angle(state) == doctest::Approx(-M_PI / 2.0));

    // triangle (0, 0), (56, 15), (15, 56) is almost equilateral
    state.fingers.clear();
    state.fingers[0] = finger_2p(0, 0, 56, 15);
    state.fingers[1] = finger_2p(56, 15, 15, 56);
    state.fingers[2] = finger_2p(15, 56, 0, 0);
    CHECK(get_rotation_angle(state) ==
        doctest::Approx(2.0 * M_PI / 3.0).epsilon(0.05));
}

TEST_CASE("finger_t")
{
    CHECK(finger_in_dir(1, 1).delta() == point_t{1, 1});
}

TEST_CASE("gesture_state_t")
{
    gesture_state_t state;
    state.fingers[0] = finger_in_dir(1, 2);
    state.fingers[1] = finger_in_dir(3, 4);
    state.fingers[2] = finger_in_dir(5, 6);
    compare_finger(state.get_center(), finger_in_dir(3, 4));
}

class action_test_t : public gesture_action_t
{
  public:
    action_test_t() {}
    bool update_state(const gesture_state_t& state) override
    {
        return false;
    }

    uint32_t get_start_time()
    {
        return this->start_time;
    }
};

TEST_CASE("gesture_action_t")
{
    action_test_t test;
    test.set_threshold(150.0);
    test.set_move_tolerance(5.0);
    test.set_duration(100.0);

    CHECK(test.get_threshold() == 150.0);
    CHECK(test.get_move_tolerance() == 5.0);
    CHECK(test.get_duration() == 100.0);

    test.reset_state(15);
    CHECK(test.get_start_time() == 15);
}
