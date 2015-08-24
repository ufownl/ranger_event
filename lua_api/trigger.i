%{
#include <ranger/event/trigger.hpp>
#include <memory>
%}

%include "swiglua_ref.i"
%include "ranger/event/trigger.hpp"

%extend ranger::event::trigger {
    void set_event_handler(SWIGLUA_REF fn) {
        auto ref = std::make_shared<swiglua_ref>(fn);
        self->set_event_handler([ref] (ranger::event::trigger& self) {
            ref->get();
            SWIG_NewPointerObj(ref->L(), &self, SWIGTYPE_p_ranger__event__trigger, 0);
            lua_call(ref->L(), 1, 0);
        });
    }
}
