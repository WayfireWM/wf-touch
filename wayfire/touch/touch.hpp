#pragma once

#include <glm/vec2.hpp>
#include <map>

namespace wf
{
namespace touch
{
using point_t = glm::dvec2;

struct finger_t
{
    point_t origin;
    point_t current;

    /** Get movement vector */
    point_t delta() const;
};

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

/**
 * Contains all fingers.
 */
struct gesture_state_t
{
  public:
    // finger_id -> finger_t
    std::map<int, finger_t> fingers;

    /** Time of last event. */
    uint32_t last_event_ms;

    /**
     * Find the center of the fingers.
     */
    finger_t get_center() const;
};

enum gesture_event_type_t
{
    EVENT_TYPE_TOUCH_DOWN,
    EVENT_TYPE_TOUCH_UP,
    EVENT_TYPE_MOTION,
};

/**
 * Represents a part of the gesture.
 */
class gesture_action_t
{
  public:
    /**
     * Set the move tolerance.
     * This is the maximum amount the fingers may move in unwanted directions.
     */
    void set_move_tolerance(double tolerance);

    /** @return The move tolerance. */
    double get_move_tolerance() const;

    /**
     * Set the threshold for the action (in pixels or radians).
     * This is the amount of movement in the desired direction(s) needed for
     * the action to be completed.
     */
    void set_threshold(double threshold);

    /** @return The threshold */
    double get_threshold() const;

    /**
     * Set the duration of the action in milliseconds.
     * This is the maximal time needed for this action to be happening to
     * consider it complete.
     */
    void set_duration(double duration);

    /** @return The duration of the gesture action. */
    double get_duration() const;

    /**
     * Update the action's state according to the next state. Note that in
     * case of a touch up event, the state still contains the to-be-removed
     * touch point.
     *
     * @param type The type of the event causing this update.
     *
     * @return True if the action is completed, false otherwise.
     */
    virtual bool update_state(const gesture_state_t& state,
        gesture_event_type_t type) = 0;

    /**
     * Reset the action's state.
     * Called when the action is cancelled or started again.
     *
     * @param time The time in milliseconds of the event triggering this.
     */
    virtual void reset_state(uint32_t time);

    virtual ~gesture_action_t() {}

  protected:
    gesture_action_t() {}
    uint32_t start_time;

  private:
    double tolerance;
    double threshold;
    double duration;
};

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
     * @param cnt_fingers The number of fingers this touch action matches.
     * @param touch_down Whether the action is touch down or touch up.
     */
    touch_action_t(int cnt_fingers, bool touch_down);

    /**
     * Set the target area of this gesture.
     */
    void set_target(const touch_target_t& target);

    /**
     * Mark the action as completed iff state has the right amount of fingers
     * and if the event is a touch down.
     */
    bool update_state(const gesture_state_t& state,
        gesture_event_type_t type) override;

  private:
    int cnt_fingers;
    gesture_event_type_t type;

    touch_target_t target;
};



}
}
