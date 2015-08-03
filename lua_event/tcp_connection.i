%{
#include <event/tcp_connection.hpp>
#include <memory>
%}

%include "swiglua_ref.i"
%include "event/tcp_connection.hpp"

%extend ranger::event::tcp_connection {
    void set_timeouts(double rd_sec, double wt_sec) {
        self->set_timeouts(std::chrono::duration<double>(rd_sec), std::chrono::duration<double>(wt_sec));
    }

    void set_event_handler(SWIGLUA_REF fn) {
        auto ref = std::make_shared<swiglua_ref>(fn);
        self->set_event_handler([ref] (  ranger::event::tcp_connection& self,
                                        ranger::event::tcp_connection::event_code what) {
            ref->get();
            SWIG_NewPointerObj(ref->L(), &self, SWIGTYPE_p_ranger__event__tcp_connection, 0);
            lua_pushnumber(ref->L(), static_cast<lua_Number>(what));
            lua_call(ref->L(), 2, 0);
        });
    }
}
