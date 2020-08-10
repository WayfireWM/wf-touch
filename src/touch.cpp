#include <wayfire/touch/touch.hpp>
#include "math.hpp"

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

void wf::touch::gesture_action_t::set_move_tolerance(double tolerance)
{
    this->tolerance = tolerance;
}

double wf::touch::gesture_action_t::get_move_tolerance() const
{
    return this->tolerance;
}

void wf::touch::gesture_action_t::set_threshold(double threshold)
{
    this->threshold = threshold;
}

double wf::touch::gesture_action_t::get_threshold() const
{
    return this->threshold;
}

void wf::touch::gesture_action_t::set_duration(double duration)
{
    this->duration = duration;
}

double wf::touch::gesture_action_t::get_duration() const
{
    return this->duration;
}

void wf::touch::gesture_action_t::reset_state(uint32_t time)
{
    this->start_time = time;
}

bool wf::touch::touch_target_t::contains(const point_t& pt) const
{
    return x <= pt.x && pt.x < x + width &&
        y <= pt.y && pt.y < y + height;
}
