#include "math.hpp"
#include <glm/glm.hpp>

#include <iostream>
#define _ << " " <<
#define debug(x) #x << " = " << (x)

wf::touch::touch_action_t::touch_action_t(int cnt_fingers, bool touch_down)
{
    this->cnt_fingers = cnt_fingers;
    this->type = touch_down ? EVENT_TYPE_TOUCH_DOWN : EVENT_TYPE_TOUCH_UP;

    this->target.x = -1e9;
    this->target.y = -1e9;
    this->target.width = 2e9;
    this->target.height = 2e9;
}

void wf::touch::touch_action_t::set_target(const touch_target_t& target)
{
    this->target = target;
}

static double find_max_delta(const gesture_state_t& state)
{
    double max_length = 0;
    for (auto& f : state.fingers)
    {
        max_length = std::max(max_length, glm::length(f.second.delta()));
    }

    return max_length;
}

bool wf::touch::touch_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    return find_max_delta(state) > this->get_move_tolerance();
}

void wf::touch::touch_action_t::reset(uint32_t time)
{
    gesture_action_t::reset(time);
    this->released_fingers = 0;
}

action_status_t wf::touch::touch_action_t::update_state(
    const gesture_state_t& state, const gesture_event_t& event)
{
    if (this->type != event.type)
    {
        return ACTION_STATUS_CANCELLED;
    }

    for (auto& f : state.fingers)
    {
        point_t relevant_point = (this->type == EVENT_TYPE_TOUCH_UP ?
            f.second.current : f.second.origin);
        if (!this->target.contains(relevant_point))
        {
            return ACTION_STATUS_CANCELLED;
        }
    }

    if (this->type == EVENT_TYPE_TOUCH_DOWN)
    {
        if (this->cnt_fingers < (int)state.fingers.size())
        {
            return ACTION_STATUS_CANCELLED;
        }

        return calculate_next_status(state, event,
            this->cnt_fingers > (int)state.fingers.size());
    } else
    {
        ++this->released_fingers;
        return calculate_next_status(state, event,
            this->released_fingers < this->cnt_fingers);
    }
}
