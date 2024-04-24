#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <wayfire/touch/touch.hpp>

using namespace wf::touch;

class fake_timer_t : public timer_interface_t
{
  public:
    std::vector<int32_t> requests;
    std::function<void()> last_cb;

    void set_timeout(uint32_t req, std::function<void()> cb) override
    {
        requests.push_back(req);
        last_cb = cb;
    }

    void reset() override
    {
        requests.push_back(-1);
    }
};

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

    auto _timer = std::make_unique<fake_timer_t>();
    auto timer_ptr = _timer.get();

#define REQUIRE_TIMERS(...) CHECK(timer_ptr->requests == std::vector<int32_t>{__VA_ARGS__});

    SUBCASE("Hold gesture")
    {
        gesture_t hold = gesture_builder_t()
            .action(touch_action_t(2, true).set_duration(100))
            .action(hold_action_t(200))
            .on_completed(callback1)
            .on_cancelled(callback2)
            .build();

        hold.set_timer(std::move(_timer));

        hold.reset(0);
        hold.update_state(
            gesture_event_t{ .type = EVENT_TYPE_TOUCH_DOWN, .time = 0, .finger = 0, .pos = {0, 0}});

        REQUIRE_TIMERS(100);

        SUBCASE("Timeout")
        {
            timer_ptr->last_cb();
            CHECK(hold.get_status() == ACTION_STATUS_CANCELLED);
        }

        SUBCASE("OK")
        {
            hold.update_state(
                gesture_event_t{ .type = EVENT_TYPE_TOUCH_DOWN, .time = 10, .finger = 1, .pos = {0, 0}});
            REQUIRE_TIMERS(100, -1, 200);
            timer_ptr->last_cb();
            CHECK(hold.get_status() == ACTION_STATUS_COMPLETED);
            REQUIRE_TIMERS(100, -1, 200, -1);
        }
    }

    SUBCASE("double-tap gesture")
    {
        gesture_t double_tap = gesture_builder_t()
            .action(touch_action_t(1, true).set_duration(100))
            .action(touch_action_t(1, false).set_duration(100))
            .action(touch_action_t(1, true).set_duration(100))
            .action(touch_action_t(1, false))
            .on_completed(callback1)
            .on_cancelled(callback2)
            .build();

        double_tap.set_timer(std::move(_timer));

        double_tap.reset(0);
        double_tap.update_state({.type = EVENT_TYPE_TOUCH_DOWN, .time = 0, .finger = 0, .pos = {0, 0}});
        double_tap.update_state({.type = EVENT_TYPE_TOUCH_UP, .time = 20, .finger = 0, .pos = {0, 0}});
        CHECK(double_tap.get_status() == ACTION_STATUS_RUNNING);

        SUBCASE("Success")
        {
            double_tap.reset(80);
            double_tap.update_state({.type = EVENT_TYPE_TOUCH_DOWN, .time = 80, .finger = 0, .pos = {0, 0}});
            double_tap.update_state({.type = EVENT_TYPE_TOUCH_UP, .time = 90, .finger = 0, .pos = {0, 0}});
            CHECK(completed == 1);
            CHECK(cancelled == 0);
        }

        SUBCASE("Timeout")
        {
            REQUIRE_TIMERS(100, -1, 100, -1, 100);

            timer_ptr->last_cb();
            CHECK(double_tap.get_status() == ACTION_STATUS_CANCELLED);
            CHECK(completed == 0);
            CHECK(cancelled == 1);
            REQUIRE_TIMERS(100, -1, 100, -1, 100, -1);

            double_tap.reset(150);
            CHECK(double_tap.get_status() == ACTION_STATUS_RUNNING);
        }
    }

    SUBCASE("swipe")
    {
        gesture_t swipe = gesture_builder_t()
            .action(touch_action_t(1, true))
            .action(hold_action_t(5))
            .action(drag_action_t(MOVE_DIRECTION_LEFT, 10))
            .action(hold_action_t(5))
            .action(drag_action_t(MOVE_DIRECTION_RIGHT, 10))
            .on_completed(callback1)
            .on_cancelled(callback2)
            .build();

        swipe.set_timer(std::move(_timer));
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
            timer_ptr->last_cb();
            gesture_event_t motion_left;
            motion_left.finger = 0;
            motion_left.pos = {-10, 0};
            motion_left.time = 10;
            motion_left.type = EVENT_TYPE_MOTION;
            swipe.update_state(motion_left);

            timer_ptr->last_cb();

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
                timer_ptr->last_cb();
                swipe.update_state(motion_left);
                timer_ptr->last_cb();
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
}
