%{
#include <event/signal.hpp>
%}

%include "lua_fnptr.i"
%include "event/signal.hpp"

%extend ranger::event::signal {
    void set_event_handler(SWIGLUA_REF fn) {
        self->set_event_handler([fn] (ranger::event::signal& self) mutable {
            swiglua_ref_get(&fn);
            SWIG_NewPointerObj(fn.L, &self, SWIGTYPE_p_ranger__event__signal, 0);
            lua_call(fn.L, 1, 0);
        });
    }
}
