#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <wayfire/touch/touch.hpp>

using namespace wf::touch;

TEST_CASE("wf::touch::gesture_t")
{
    int completed = 0;
    int cancelled = 0;
    gesture_callback_t callback1 = [&] ()
    {
        ++completed;
    };

    gesture_callback_t callback2 = [&] ()
    {
        ++cancelled;
    };

    gesture_t swipe = gesture_builder_t()
        .action(touch_action_t(1, true))
        .action(hold_action_t(5))
        .action(drag_action_t(MOVE_DIRECTION_LEFT, 10))
        .action(hold_action_t(5))
        .action(drag_action_t(MOVE_DIRECTION_RIGHT, 10))
        .on_completed(callback1)
        .on_cancelled(callback2)
        .build();

    swipe.reset(0);
    gesture_event_t touch_down;
    touch_down.finger = 0;
    touch_down.pos = {0, 0};
    touch_down.type = EVENT_TYPE_TOUCH_DOWN;
    touch_down.time = 0;
    swipe.update_state(touch_down);
    CHECK(swipe.get_progress() >= 0.2);

    SUBCASE("complete")
    {
        gesture_event_t motion_left;
        motion_left.finger = 0;
        motion_left.pos = {-10, 0};
        motion_left.time = 10;
        motion_left.type = EVENT_TYPE_MOTION;
        swipe.update_state(motion_left);

        CHECK(cancelled == 0);
        CHECK(completed == 0);
        CHECK(swipe.get_progress() >= 0.6);

        gesture_event_t motion_right = motion_left;
        motion_right.pos = {0, 0};
        motion_right.time = 20;
        swipe.update_state(motion_right);
        CHECK(cancelled == 0);
        CHECK(completed == 1);

        SUBCASE("restart")
        {
            swipe.reset(0);
            CHECK(swipe.get_progress() == 0.0);
            swipe.update_state(touch_down);
            swipe.update_state(motion_left);
            swipe.update_state(motion_right);
            CHECK(cancelled == 0);
            CHECK(completed == 2);
        }
    }

    SUBCASE("cancelled")
    {
        touch_down.finger = 1;
        swipe.update_state(touch_down);
        CHECK(cancelled == 1);
        CHECK(completed == 0);
    }
}
