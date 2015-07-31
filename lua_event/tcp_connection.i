%{
#include <event/tcp_connection.hpp>
%}

%include <lua_fnptr.i>
%include "event/tcp_connection.hpp"

%extend ranger::event::tcp_connection {
    void set_timeouts(double rd_sec, double wt_sec) {
        self->set_timeouts(std::chrono::duration<double>(rd_sec), std::chrono::duration<double>(wt_sec));
    }

    void set_event_handler(SWIGLUA_REF fn) {
        self->set_event_handler([fn] (  ranger::event::tcp_connection& self,
                                        ranger::event::tcp_connection::event_code what) mutable {
            swiglua_ref_get(&fn);
            SWIG_NewPointerObj(fn.L, &self, SWIGTYPE_p_ranger__event__tcp_connection, 0);
            lua_pushnumber(fn.L, static_cast<lua_Number>(what));
            lua_call(fn.L, 2, 0);
        });
    }
}
