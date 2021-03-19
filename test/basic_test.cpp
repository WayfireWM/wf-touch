#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "shared.hpp"

TEST_CASE("get_move_in_direction")
{
    CHECK(finger_in_dir(1, 0).get_direction() == MOVE_DIRECTION_RIGHT);
    CHECK(finger_in_dir(-1, -1).get_direction() == lu);
    CHECK(finger_in_dir(1, 1).get_direction() == rd);
    CHECK(finger_in_dir(0, 0).get_direction() == 0);
    CHECK(finger_in_dir(-10, 1).get_direction() == MOVE_DIRECTION_LEFT);
}

TEST_CASE("get_drag_distance")
{
    CHECK(finger_in_dir(0, 5).get_drag_distance(MOVE_DIRECTION_DOWN)
        == doctest::Approx(5));
    CHECK(finger_in_dir(-1, -1).get_drag_distance(MOVE_DIRECTION_DOWN)
        == doctest::Approx(0));
}

TEST_CASE("get_incorrect_drag_distance")
{
    CHECK(finger_in_dir(-1, -1).get_incorrect_drag_distance(lu) ==
        doctest::Approx(0));
    CHECK(finger_in_dir(-1, -1).get_incorrect_drag_distance(ru) ==
        doctest::Approx(std::sqrt(2)));
    CHECK(finger_in_dir(-1, -1).get_incorrect_drag_distance(ld) ==
        doctest::Approx(std::sqrt(2)));
    CHECK(finger_in_dir(5, 5).get_incorrect_drag_distance(MOVE_DIRECTION_RIGHT)
        == doctest::Approx(5));
    CHECK(finger_in_dir(4, 0).get_incorrect_drag_distance(MOVE_DIRECTION_LEFT)
        == doctest::Approx(4));
}

TEST_CASE("get_pinch_scale")
{
    gesture_state_t state;
    state.fingers[0] = finger_2p(1, 0, 2, 1);
    state.fingers[1] = finger_2p(-1, -2, -3, -4);
    CHECK(state.get_pinch_scale() > 2);

    std::swap(state.fingers[0].origin, state.fingers[0].current);
    std::swap(state.fingers[1].origin, state.fingers[1].current);
    CHECK(state.get_pinch_scale() < 0.5);

    state.fingers[0] = finger_2p(1, 1, 1, 1);
    state.fingers[1] = finger_2p(2, 2, 2, 2);
    CHECK(state.get_pinch_scale() == doctest::Approx(1));
}

TEST_CASE("get_rotation_angle")
{
    gesture_state_t state;
    state.fingers[0] = finger_2p(0, 1, 1, 0);
    state.fingers[1] = finger_2p(1, 0, 0, -1);
    state.fingers[2] = finger_2p(0, -1, -1, 0);
    state.fingers[3] = finger_2p(-1, 0, 0, 1);
    CHECK(state.get_rotation_angle() == doctest::Approx(-M_PI / 2.0));

    // triangle (0, 0), (56, 15), (15, 56) is almost equilateral
    state.fingers.clear();
    state.fingers[0] = finger_2p(0, 0, 56, 15);
    state.fingers[1] = finger_2p(56, 15, 15, 56);
    state.fingers[2] = finger_2p(15, 56, 0, 0);
    CHECK(state.get_rotation_angle() ==
        doctest::Approx(2.0 * M_PI / 3.0).epsilon(0.05));
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

TEST_CASE("gesture_state_t::update")
{
    gesture_state_t state;

    gesture_event_t ev;
    ev.finger = 0;
    ev.pos = {4, 5};
    ev.type = EVENT_TYPE_TOUCH_DOWN;
    state.update(ev);
    CHECK(state.fingers.size() == 1);
    compare_finger(state.fingers[0], finger_2p(4, 5, 4, 5));

    ev.finger = 0;
    ev.pos = {6, 7};
    ev.type = EVENT_TYPE_MOTION;
    state.update(ev);
    CHECK(state.fingers.size() == 1);
    compare_finger(state.fingers[0], finger_2p(4, 5, 6, 7));

    ev.finger = 1;
    ev.pos = {7, -1};
    ev.type = EVENT_TYPE_TOUCH_DOWN;
    state.update(ev);
    CHECK(state.fingers.size() == 2);
    compare_finger(state.fingers[0], finger_2p(4, 5, 6, 7));
    compare_finger(state.fingers[1], finger_2p(7, -1, 7, -1));

    ev.type = EVENT_TYPE_TOUCH_UP;
    ev.finger = 0;
    state.update(ev);
    CHECK(state.fingers.size() == 1);
    compare_finger(state.fingers[1], finger_2p(7, -1, 7, -1));
}

TEST_CASE("gesture_state_t::reset_origin")
{
    gesture_state_t state;
    state.fingers[0] = finger_in_dir(6, 7);
    state.reset_origin();
    CHECK(state.fingers.size() == 1);
    compare_finger(state.fingers[0], finger_2p(6, 7, 6, 7));
}

class action_test_t : public gesture_action_t
{
  public:
    action_test_t() {}
    uint32_t get_start_time()
    {
        return this->start_time;
    }

    virtual action_status_t update_state(const gesture_state_t& state,
        const gesture_event_t& event)
    {
        return calculate_next_status(state, event, true);
    }
};

TEST_CASE("gesture_action_t")
{
    action_test_t test;
    test.set_move_tolerance(5.0);
    test.set_duration(20);

    CHECK(test.get_move_tolerance() == 5.0);
    CHECK(test.get_duration() == 20);

    gesture_state_t state;
    gesture_event_t event;
    event.time = 20;
    test.reset(0);
    CHECK(test.update_state(state, event) == ACTION_STATUS_RUNNING);
    event.time = 21;
    CHECK(test.update_state(state, event) == ACTION_STATUS_CANCELLED);
}

TEST_CASE("touch_target_t")
{
    touch_target_t target{-1, 1, 2, 2};
    CHECK(target.contains({0, 2}));
    CHECK(target.contains({-1, 1}));
    CHECK(!target.contains({1, 3}));
    CHECK(!target.contains({0, 5}));
}
