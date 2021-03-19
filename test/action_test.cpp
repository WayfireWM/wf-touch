#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "shared.hpp"

TEST_CASE("touch_action_t")
{
    touch_action_t touch_down{2, true};
    touch_down.set_target({0, 0, 10, 10});
    touch_down.set_duration(150);
    touch_down.set_move_tolerance(5);

    gesture_event_t event_down;
    event_down.type = EVENT_TYPE_TOUCH_DOWN;
    event_down.time = 75;

    // check normal operation, with tolerance
    gesture_state_t state;
    state.fingers[0] = finger_2p(0, 0, 0, 0);
    touch_down.reset(0);
    CHECK(touch_down.update_state(state, event_down) == ACTION_STATUS_RUNNING);

    gesture_event_t motion;
    motion.type = EVENT_TYPE_MOTION;
    motion.finger = 0;
    motion.time = 100;
    motion.pos = {1, 1};
    state.fingers[0] = finger_2p(0, 0, 1, 1);
    CHECK(touch_down.update_state(state, motion) == ACTION_STATUS_RUNNING);

    state.fingers[1] = finger_in_dir(2, 2);
    event_down.finger = 2;
    event_down.pos = {2, 2};
    event_down.time = 150;
    CHECK(touch_down.update_state(state, event_down) == ACTION_STATUS_COMPLETED);

    // check outside of bounds
    state.fingers[0] = finger_2p(15, 15, 20, 20);
    touch_down.reset(0);
    CHECK(touch_down.update_state(state, event_down) == ACTION_STATUS_CANCELLED);
    state.fingers[0] = finger_2p(0, 0, 0, 0);

    // check timeout
    event_down.time = 151;
    touch_down.reset(0);
    CHECK(touch_down.update_state(state, event_down) == ACTION_STATUS_CANCELLED);

    touch_action_t touch_up{2, false};
    gesture_event_t event_up;
    event_up.type = EVENT_TYPE_TOUCH_UP;
    event_up.time = 150;

    // start touch up action
    state.fingers[1] = finger_2p(2, 2, 3, 3);
    touch_up.reset(0);
    CHECK(touch_up.update_state(state, event_up) == ACTION_STATUS_RUNNING);

    // complete it
    state.fingers.erase(1);
    CHECK(touch_up.update_state(state, event_up) == ACTION_STATUS_COMPLETED);

    // check tolerance exceeded
    state.fingers[1] = finger_2p(2, 2, 2, 3);
    touch_up.set_move_tolerance(0);
    touch_up.reset(0);
    CHECK(touch_up.update_state(state, event_up) == ACTION_STATUS_CANCELLED);
}

TEST_CASE("wf::touch::hold_action_t")
{
    hold_action_t hold{50};
    hold.set_move_tolerance(1);

    gesture_state_t state;
    state.fingers[0] = finger_in_dir(1, 0);
    gesture_event_t ev;

    // check ok state
    hold.reset(0);
    ev.time = 49;
    ev.type = EVENT_TYPE_MOTION;
    CHECK(hold.update_state(state, ev) == ACTION_STATUS_RUNNING);
    ev.time = 50;
    ev.type = EVENT_TYPE_TOUCH_UP;
    CHECK(hold.update_state(state, ev) == ACTION_STATUS_ALREADY_COMPLETED);

    // check finger breaks action
    hold.reset(0);
    ev.type = EVENT_TYPE_TOUCH_UP;
    ev.time = 49;
    CHECK(hold.update_state(state, ev) == ACTION_STATUS_CANCELLED);

    // check too much movement
    state.fingers[0] = finger_in_dir(2, 0);
    ev.time = 49;
    hold.reset(0);
    CHECK(hold.update_state(state, ev) == ACTION_STATUS_CANCELLED);
}

TEST_CASE("wf::touch::drag_action_t")
{
    drag_action_t drag{MOVE_DIRECTION_LEFT, 50};
    drag.set_move_tolerance(5);

    gesture_state_t state;
    state.fingers[0] = finger_in_dir(-50, 0);
    state.fingers[1] = finger_in_dir(-50, 3);

    gesture_event_t ev;
    ev.type = EVENT_TYPE_MOTION;
    ev.time = 0;

    // check ok
    drag.reset(0);
    CHECK(drag.update_state(state, ev) == ACTION_STATUS_COMPLETED);

    // check distance not enough
    drag.reset(0);
    state.fingers[0] = finger_in_dir(-49, 0);
    CHECK(drag.update_state(state, ev) == ACTION_STATUS_RUNNING);

    // check exceeds tolerance
    state.fingers[1] = finger_in_dir(0, 6);
    drag.reset(0);
    CHECK(drag.update_state(state, ev) == ACTION_STATUS_CANCELLED);

    // check touch cancels
    ev.type = EVENT_TYPE_TOUCH_UP;
    state.fingers[1] = finger_in_dir(-50, 3);
    drag.reset(0);
    CHECK(drag.update_state(state, ev) == ACTION_STATUS_CANCELLED);
}

TEST_CASE("wf::touch::pinch_action_t")
{
    pinch_action_t in{0.5}, out{2};

    gesture_state_t state;
    state.fingers[0] = finger_2p(1, 0, 2, 1);
    state.fingers[1] = finger_2p(-1, -2, -3, -4);

    gesture_event_t ev;
    ev.time = 0;
    ev.type = EVENT_TYPE_MOTION;

    // ok
    out.reset(0);
    CHECK(out.update_state(state, ev) == ACTION_STATUS_COMPLETED);

    std::swap(state.fingers[0].origin, state.fingers[0].current);
    std::swap(state.fingers[1].origin, state.fingers[1].current);
    in.reset(0);
    CHECK(in.update_state(state, ev) == ACTION_STATUS_COMPLETED);

    // too much movement
    in.set_move_tolerance(1);
    in.reset(0);
    state.fingers[0].current += point_t{2, 0};
    state.fingers[1].current += point_t{2, 0};
    CHECK(in.update_state(state, ev) == ACTION_STATUS_CANCELLED);

    // touch cancels
    in.reset(0);
    state.fingers[0].current -= point_t{2, 0};
    state.fingers[1].current -= point_t{2, 0};
    ev.type = EVENT_TYPE_TOUCH_DOWN;
    CHECK(in.update_state(state, ev) == ACTION_STATUS_CANCELLED);
}

TEST_CASE("wf::touch::rotate_action_t")
{
    gesture_state_t state;
    state.fingers[0] = finger_2p(0, 1, 1, 0);
    state.fingers[1] = finger_2p(1, 0, 0, -1);
    state.fingers[2] = finger_2p(0, -1, -1, 0);
    state.fingers[3] = finger_2p(-1, 0, 0, 1);
    CHECK(state.get_rotation_angle() == doctest::Approx(-M_PI / 2.0));

    rotate_action_t rotate{-M_PI / 3.0};
    gesture_event_t ev;
    ev.type = EVENT_TYPE_MOTION;
    CHECK(rotate.update_state(state, ev) == ACTION_STATUS_COMPLETED);

    // TODO: incomplete tests
}
