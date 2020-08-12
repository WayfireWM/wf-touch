#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "shared.hpp"

TEST_CASE("touch_action_t")
{
    touch_action_t touch_down{1, true};
    touch_down.set_target({0, 0, 10, 10});
    touch_down.set_duration(150);

    gesture_event_t event_down;
    event_down.type = EVENT_TYPE_TOUCH_DOWN;
    event_down.time = 150;

    // check normal operation
    gesture_state_t state;
    state.fingers[0] = finger_2p(0, 0, 1, 1);
    touch_down.reset(0);
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

    // check tolerance
    state.fingers[1] = finger_2p(2, 2, 2, 3);
    touch_up.set_move_tolerance(0);
    touch_up.reset(0);
    CHECK(touch_up.update_state(state, event_up) == ACTION_STATUS_CANCELLED);
}
