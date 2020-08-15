#include <wayfire/touch/touch.hpp>

#include <iostream>
#define _ << " " <<
#define debug(x) #x << " = " << (x)

using namespace wf::touch;

point_t wf::touch::finger_t::delta() const
{
    return this->current - this->origin;
}

finger_t wf::touch::gesture_state_t::get_center() const
{
    finger_t center;
    center.origin = {0, 0};
    center.current = {0, 0};

    for (auto& f : this->fingers)
    {
        center.origin += f.second.origin;
        center.current += f.second.current;
    }

    center.origin /= this->fingers.size();
    center.current /= this->fingers.size();
    return center;
}

void wf::touch::gesture_state_t::update(const gesture_event_t& event)
{
    switch (event.type)
    {
      case EVENT_TYPE_TOUCH_DOWN:
        fingers[event.finger].origin = {event.x, event.y};
        // fallthrough
      case EVENT_TYPE_MOTION:
        fingers[event.finger].current = {event.x, event.y};
        break;
      case EVENT_TYPE_TOUCH_UP:
        fingers.erase(event.finger);
        break;
    }
}

void wf::touch::gesture_state_t::reset_origin()
{
    for (auto& f : fingers)
    {
        f.second.origin = f.second.current;
    }
}

void wf::touch::gesture_action_t::set_move_tolerance(double tolerance)
{
    this->tolerance = tolerance;
}

double wf::touch::gesture_action_t::get_move_tolerance() const
{
    return this->tolerance;
}

void wf::touch::gesture_action_t::set_duration(uint32_t duration)
{
    this->duration = duration;
}

uint32_t wf::touch::gesture_action_t::get_duration() const
{
    return this->duration;
}

action_status_t wf::touch::gesture_action_t::calculate_next_status(
    const gesture_state_t& state, const gesture_event_t& last_event, bool running)
{
    uint32_t elapsed = last_event.time - this->start_time;
    if ((elapsed > this->get_duration()) || exceeds_tolerance(state))
    {
        return ACTION_STATUS_CANCELLED;
    }

    return running ? ACTION_STATUS_RUNNING : ACTION_STATUS_COMPLETED;
}

bool wf::touch::gesture_action_t::exceeds_tolerance(const gesture_state_t& state)
{
    return false;
}

void wf::touch::gesture_action_t::reset(uint32_t time)
{
    this->start_time = time;
}

bool wf::touch::touch_target_t::contains(const point_t& pt) const
{
    return x <= pt.x && pt.x < x + width &&
        y <= pt.y && pt.y < y + height;
}
