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
