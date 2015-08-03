%{
#include <event/signal.hpp>
#include <memory>
%}

%include "swiglua_ref.i"
%include "event/signal.hpp"

%extend ranger::event::signal {
    void set_event_handler(SWIGLUA_REF fn) {
        auto ref = std::make_shared<swiglua_ref>(fn);
        self->set_event_handler([ref] (ranger::event::signal& self) {
            ref->get();
            SWIG_NewPointerObj(ref->L(), &self, SWIGTYPE_p_ranger__event__signal, 0);
            lua_call(ref->L(), 1, 0);
        });
    }
}
