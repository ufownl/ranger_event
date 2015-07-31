%{
#include <event/tcp_acceptor.hpp>
%}

%include <lua_fnptr.i>
%include "event/tcp_acceptor.hpp"

%extend ranger::event::tcp_acceptor {
    void set_event_handler(SWIGLUA_REF fn) {
        self->set_event_handler([fn] (ranger::event::tcp_acceptor& self, int fd) mutable {
            swiglua_ref_get(&fn);
            SWIG_NewPointerObj(fn.L, &self, SWIGTYPE_p_ranger__event__tcp_acceptor, 0);
            lua_pushnumber(fn.L, static_cast<lua_Number>(fd));
            lua_call(fn.L, 2, 1);
            return lua_toboolean(fn.L, -1) ? true : false;
        });
    }
}
