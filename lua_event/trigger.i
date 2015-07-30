%{
#include <event/trigger.hpp>
%}

%include "lua_fnptr.i"
%include "event/trigger.hpp"

%extend ranger::event::trigger {
    void set_event_handler(SWIGLUA_REF fn) {
        self->set_event_handler([fn] (ranger::event::trigger& self) mutable {
            swiglua_ref_get(&fn);
            SWIG_NewPointerObj(fn.L, &self, SWIGTYPE_p_ranger__event__trigger, 0);
            lua_call(fn.L, 1, 0);
        });
    }
}
