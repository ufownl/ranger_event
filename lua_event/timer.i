%{
#include <event/timer.hpp>
#include <memory>
%}

%include "swiglua_ref.i"
%include "event/timer.hpp"

%extend ranger::event::timer {
    void set_event_handler(SWIGLUA_REF fn) {
        auto ref = std::make_shared<swiglua_ref>(fn);
        self->set_event_handler([ref] (ranger::event::timer& self) {
            ref->get();
            SWIG_NewPointerObj(ref->L(), &self, SWIGTYPE_p_ranger__event__timer, 0);
            lua_call(ref->L(), 1, 0);
        });
    }

    void active(double sec) {
        self->active(std::chrono::duration<double>(sec));
    }
}
