#pragma once

/**
 * Touchscreen gesture library, designed for use in Wayfire (and elsewhere).
 * Goal is to process touch events and detect various configurable gestures.
 *
 * High-level design:
 * A gesture consists of one or more consecutive actions.
 *
 * An action is usually a simple part of the gesture which can be processed
 * separately, for ex. touch down with 3 fingers, swipe in a direction, etc.
 *
 * When processing events, the gesture starts with its first action. Once it is
 * completed, the processing continues with the next action, and so on, until
 * either all actions are completed or an action cancels the gesture.
 */
#include <glm/vec2.hpp>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <optional>

namespace wf
{
namespace touch
{
using point_t = glm::dvec2;


/**
 * Movement direction.
 */
enum move_direction_t
{
    MOVE_DIRECTION_LEFT  = (1 << 0),
    MOVE_DIRECTION_RIGHT = (1 << 1),
    MOVE_DIRECTION_UP    = (1 << 2),
    MOVE_DIRECTION_DOWN  = (1 << 3),
};

struct finger_t
{
    point_t origin;
    point_t current;

    /** Get movement vector */
    point_t delta() const;

    /** Find direction of movement, a bitmask of move_direction_t */
    uint32_t get_direction() const;

    /** Find drag distance in the given direction */
    double get_drag_distance(uint32_t direction) const;

    /** Find drag distance in opposite and perpendicular directions */
    double get_incorrect_drag_distance(uint32_t direction) const;
};

enum gesture_event_type_t
{
    /** Finger touched down the screen */
    EVENT_TYPE_TOUCH_DOWN,
    /** Finger was lifted off the screen */
    EVENT_TYPE_TOUCH_UP,
    /** Finger moved across the screen */
    EVENT_TYPE_MOTION,
    /** Timeout since action start */
    EVENT_TYPE_TIMEOUT,
};

/**
 * Represents a single update on the touch state.
 */
struct gesture_event_t
{
    /** type of the event */
    gesture_event_type_t type;
    /** timestamp of the event in milliseconds */
    uint32_t time{};
    /** finger id which the event is about */
    int32_t finger{};

    /** coordinates of the finger */
    point_t pos{};
};

/**
 * Contains all fingers.
 */
struct gesture_state_t
{
  public:
    // finger_id -> finger_t
    std::map<int, finger_t> fingers;

    /** Update fingers based on the event */
    void update(const gesture_event_t& event);

    /** Reset finger origin to current positions */
    void reset_origin();

    /** Find the center points of the fingers. */
    finger_t get_center() const;

    /** Get the pinch scale of current touch points. */
    double get_pinch_scale() const;

    /**
     * Get the rotation angle in radians of current touch points.
     * NB: Works only for rotation < 180 degrees.
     */
    double get_rotation_angle() const;
};

/**
 * Represents the status of an action after it is updated
 */
enum action_status_t
{
    /** Action is done after this event. */
    ACTION_STATUS_COMPLETED,
    /** Action is still running after this event. */
    ACTION_STATUS_RUNNING,
    /** The whole gesture should be cancelled. */
    ACTION_STATUS_CANCELLED,
};

/**
 * Represents a part of the gesture.
 */
class gesture_action_t
{
  public:
    /**
     * Set the duration of the action in milliseconds.
     *
     * After the duration times out, the action will receive
     *
     *
     * This is the maximal time needed for this action to be happening to
     * consider it complete.
     *
     * @return this
     */
    gesture_action_t& set_duration(uint32_t duration);

    /** @return The duration of the gesture action. */
    std::optional<uint32_t> get_duration() const;

    /**
     * Update the action's state according to the new state.
     *
     * NOTE: The actual implementation should update the @start_time field.
     *
     * @param state The gesture state since the last reset of the gesture.
     * @param event The event causing this update.
     * @return The new action status.
     */
    virtual action_status_t update_state(const gesture_state_t& state,
        const gesture_event_t& event) = 0;

    /**
     * Reset the action.
     * Called whenever the action is started again.
     */
    virtual void reset(uint32_t time);

    virtual ~gesture_action_t() {}

  protected:
    gesture_action_t() {}

    /** Time of the first event. */
    int64_t start_time;

  private:
    std::optional<uint32_t> duration; // maximal duration
};

#define WFTOUCH_BUILDER_REPEAT_MEMBERS_WITH_CAST(x) \
    x& set_move_tolerance(double tolerance) \
    { \
        this->move_tolerance = tolerance; \
        return *this; \
    } \
    x& set_duration(uint32_t duration) \
    { \
        gesture_action_t::set_duration(duration); \
        return *this; \
    }

/**
 * Represents a target area where the touch event takes place.
 */
struct touch_target_t
{
    double x;
    double y;
    double width;
    double height;

    bool contains(const point_t& point) const;
};

/**
 * Represents the action of touching down with several fingers.
 */
class touch_action_t : public gesture_action_t
{
  public:
    /**
     * Create a new touch down or up action.
     *
     * @param cnt_fingers The number of fingers that need to be touched down
     *   or released to consider the action completed.
     * @param touch_down Whether the action is touch down or touch up.
     */
    touch_action_t(int cnt_fingers, bool touch_down);
    WFTOUCH_BUILDER_REPEAT_MEMBERS_WITH_CAST(touch_action_t);

    /**
     * Set the target area of this gesture.
     *
     * @return this
     */
    touch_action_t& set_target(const touch_target_t& target);

    /**
     * Mark the action as completed iff state has the right amount of fingers
     * and if the event is a touch down.
     */
    action_status_t update_state(const gesture_state_t& state,
        const gesture_event_t& event) override;

    void reset(uint32_t time) override;

  protected:
    /** @return True if the fingers have moved too much. */
    bool exceeds_tolerance(const gesture_state_t& state);

  private:
    int cnt_fingers;
    int cnt_touch_events;
    gesture_event_type_t type;
    uint32_t move_tolerance = 1e9;

    touch_target_t target;
};

/**
 * Represents the action of holding the fingers still for a certain amount
 * of time.
 */
class hold_action_t : public gesture_action_t
{
  public:
    /**
     * Create a new hold action.
     *
     * @param threshold The time is milliseconds needed to consider the gesture
     *   complete.
     */
    hold_action_t(int32_t threshold);
    WFTOUCH_BUILDER_REPEAT_MEMBERS_WITH_CAST(hold_action_t);

    /**
     * The action is already completed iff no fingers have been added or
     * released and the given amount of time has passed without much movement.
     */
    action_status_t update_state(const gesture_state_t& state,
        const gesture_event_t& event) override;

  protected:
    /** @return True if the fingers have moved too much. */
    bool exceeds_tolerance(const gesture_state_t& state);

  private:
    uint32_t move_tolerance = 1e9;
};

/**
 * Represents the action of dragging the fingers in a particular direction
 * over a particular distance.
 */
class drag_action_t : public gesture_action_t
{
  public:
    /**
     * Create a new drag action.
     *
     * @param direction The direction of the drag action.
     * @param threshold The distance that needs to be covered.
     */
    drag_action_t(uint32_t direction, double threshold);
    WFTOUCH_BUILDER_REPEAT_MEMBERS_WITH_CAST(drag_action_t);

    /**
     * The action is already completed iff no fingers have been added or
     * released and the given amount of time has passed without much movement.
     */
    action_status_t update_state(const gesture_state_t& state,
        const gesture_event_t& event) override;

  protected:
    /**
     * @return True if any finger has moved more than the threshold in an
     *  incorrect direction.
     */
    bool exceeds_tolerance(const gesture_state_t& state);

  private:
    double threshold;
    uint32_t direction;
    uint32_t move_tolerance = 1e9;
};

/**
 * Represents a pinch action.
 */
class pinch_action_t : public gesture_action_t
{
  public:
    /**
     * Create a new pinch action.
     *
     * @param threshold The threshold to be exceeded.
     *   If threshold is less/more than 1, then the action is complete when
     *   the actual pinch scale is respectively less/more than threshold.
     */
    pinch_action_t(double threshold);
    WFTOUCH_BUILDER_REPEAT_MEMBERS_WITH_CAST(pinch_action_t);

    /**
     * The action is already completed iff no fingers have been added or
     * released and the pinch threshold has been reached without much movement.
     */
    action_status_t update_state(const gesture_state_t& state,
        const gesture_event_t& event) override;

  protected:
    /**
     * @return True if gesture center has moved more than tolerance.
     */
    bool exceeds_tolerance(const gesture_state_t& state);

  private:
    double threshold;
    uint32_t move_tolerance = 1e9;
};

/**
 * Represents a rotate action.
 */
class rotate_action_t : public gesture_action_t
{
  public:
    /**
     * Create a new rotate action.
     *
     * @param threshold The threshold to be exceeded.
     *   If threshold is less/more than 0, then the action is complete when
     *   the actual rotation angle is respectively less/more than threshold.
     */
    rotate_action_t(double threshold);
    WFTOUCH_BUILDER_REPEAT_MEMBERS_WITH_CAST(rotate_action_t);

    /**
     * The action is already completed iff no fingers have been added or
     * released and the rotation threshold has been reached without much movement.
     */
    action_status_t update_state(const gesture_state_t& state,
        const gesture_event_t& event) override;

  protected:
    /**
     * @return True if gesture center has moved more than tolerance.
     */
    bool exceeds_tolerance(const gesture_state_t& state);

  private:
    double threshold;
    uint32_t move_tolerance = 1e9;
};

using gesture_callback_t = std::function<void()>;

class timer_interface_t
{
  public:
    virtual void set_timeout(uint32_t msec, std::function<void()> handler) = 0;
    virtual void reset() = 0;
    virtual ~timer_interface_t() = default;
};

/**
 * Represents a series of actions forming a gesture together.
 */
class gesture_t
{
  public:
    /**
     * Create a new gesture consisting of the given actions.
     *
     * @param actions The actions the gesture consists of.
     * @param completed The callback to execute each time the gesture is
     *   completed.
     * @param cancelled The callback to execute each time the gesture is
     *   cancelled.
     */
    gesture_t(std::vector<std::unique_ptr<gesture_action_t>> actions = {},
        gesture_callback_t completed = [](){}, gesture_callback_t cancelled = [](){});

    gesture_t(gesture_t&& other);
    gesture_t& operator=(gesture_t&& other);

    ~gesture_t();

    /** @return What percentage of the actions are complete. */
    double get_progress() const;

    /**
     * Update the gesture state.
     *
     * @param event The next event.
     */
    void update_state(const gesture_event_t& event);

    /**
     * Get the current state of the gesture.
     */
    action_status_t get_status() const;

    /**
     * Reset the gesture state.
     *
     * @param time The time of the event causing the start of gesture
     *   recognition, this is typically the first touch event.
     */
    void reset(uint32_t time);

    /**
     * Set the timer to use for the gesture.
     * This needs to be called before using the gesture.
     *
     * In wayfire, this is usually set by core.
     */
    void set_timer(std::unique_ptr<timer_interface_t> timer);

  private:
    class impl;
    std::unique_ptr<impl> priv;
};

/**
 * A helper class to facilitate gesture construction.
 */
class gesture_builder_t
{
  public:
    gesture_builder_t();

    template<class ActionType>
    gesture_builder_t& action(const ActionType& action)
    {
        actions.push_back(std::make_unique<ActionType>(action));
        return *this;
    }

    gesture_builder_t& on_completed(gesture_callback_t callback);
    gesture_builder_t& on_cancelled(gesture_callback_t callback);
    gesture_t build();

  private:
    gesture_callback_t _on_completed = [](){};
    gesture_callback_t _on_cancelled = [](){};
    std::vector<std::unique_ptr<gesture_action_t>> actions;
};
}
}
