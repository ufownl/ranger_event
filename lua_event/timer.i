%{
#include <event/timer.hpp>
%}

%include <lua_fnptr.i>
%include "event/timer.hpp"

%extend ranger::event::timer {
    void set_event_handler(SWIGLUA_REF fn) {
        self->set_event_handler([fn] (ranger::event::timer& self) mutable {
            swiglua_ref_get(&fn);
            SWIG_NewPointerObj(fn.L, &self, SWIGTYPE_p_ranger__event__timer, 0);
            lua_call(fn.L, 1, 0);
        });
    }

    void active(double sec) {
        self->active(std::chrono::duration<double>(sec));
    }
}
