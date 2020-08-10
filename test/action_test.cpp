#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "shared.hpp"

TEST_CASE("touch_action_t")
{
    touch_action_t touch_down{1, true};
    touch_down.set_target({0, 0, 10, 10});

    gesture_state_t state;
    state.fingers[0] = finger_in_dir(50, 50);
    CHECK(touch_down.update_state(state, EVENT_TYPE_TOUCH_DOWN));
    touch_down.reset_state(0);
    state.fingers[0] = finger_2p(10, 10, 5, 5);
    CHECK(!touch_down.update_state(state, EVENT_TYPE_TOUCH_DOWN));

    touch_action_t touch_up{2, false};
    CHECK(!touch_up.update_state(state, EVENT_TYPE_TOUCH_UP));

    state.fingers[1] = finger_2p(10, 10, 5, 5);
    CHECK(!touch_up.update_state(state, EVENT_TYPE_TOUCH_DOWN));
    CHECK(touch_up.update_state(state, EVENT_TYPE_TOUCH_UP));

    touch_up.reset_state(0);
    touch_up.set_target({0, 0, 7, 7});
    CHECK(touch_up.update_state(state, EVENT_TYPE_TOUCH_UP));

    touch_up.reset_state(0);
    touch_up.set_target({0, 0, 3, 3});
    CHECK(!touch_up.update_state(state, EVENT_TYPE_TOUCH_UP));
}
