%{
#include <event/tcp_acceptor.hpp>
#include <memory>
%}

%include "swiglua_ref.i"
%include "event/tcp_acceptor.hpp"

%extend ranger::event::tcp_acceptor {
    void set_event_handler(SWIGLUA_REF fn) {
        auto ref = std::make_shared<swiglua_ref>(fn);
        self->set_event_handler([ref] (ranger::event::tcp_acceptor& self, int fd) {
            ref->get();
            SWIG_NewPointerObj(ref->L(), &self, SWIGTYPE_p_ranger__event__tcp_acceptor, 0);
            lua_pushnumber(ref->L(), static_cast<lua_Number>(fd));
            lua_call(ref->L(), 2, 1);
            return lua_toboolean(ref->L(), -1) ? true : false;
        });
    }
}
