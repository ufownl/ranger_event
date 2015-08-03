%{
#include <event/tcp_connection.hpp>
#include <memory>
%}

%include "swiglua_ref.i"

%{
class lua_filter_handler : public ranger::event::tcp_connection::filter_handler {
public:
    lua_filter_handler(const SWIGLUA_REF& input_hdl, const SWIGLUA_REF& output_hdl)
        : m_input_hdl(input_hdl)
        , m_output_hdl(output_hdl) {
        // nop
    }

    ~lua_filter_handler() {
        // nop
    }

    bool handle_input(ranger::event::buffer&& src, ranger::event::buffer&& dst) final {
        m_input_hdl.get();
        SWIG_NewPointerObj(m_input_hdl.L(), &src, SWIGTYPE_p_ranger__event__buffer, 0);
        SWIG_NewPointerObj(m_input_hdl.L(), &dst, SWIGTYPE_p_ranger__event__buffer, 0);
        lua_call(m_input_hdl.L(), 2, 1);
        return lua_toboolean(m_input_hdl.L(), -1) ? true : false;
    }

    bool handle_output(ranger::event::buffer&& src, ranger::event::buffer&& dst) final {
        m_output_hdl.get();
        SWIG_NewPointerObj(m_output_hdl.L(), &src, SWIGTYPE_p_ranger__event__buffer, 0);
        SWIG_NewPointerObj(m_output_hdl.L(), &dst, SWIGTYPE_p_ranger__event__buffer, 0);
        lua_call(m_output_hdl.L(), 2, 1);
        return lua_toboolean(m_output_hdl.L(), -1) ? true : false;
    }

private:
    swiglua_ref m_input_hdl;
    swiglua_ref m_output_hdl;
};
%}

%include "event/tcp_connection.hpp"

%extend ranger::event::tcp_connection {
    void set_timeouts(double rd_sec, double wt_sec) {
        self->set_timeouts(std::chrono::duration<double>(rd_sec), std::chrono::duration<double>(wt_sec));
    }

    void append_filter(SWIGLUA_REF input_hdl, SWIGLUA_REF output_hdl) {
        self->append_filter<lua_filter_handler>(input_hdl, output_hdl);
    }

    void set_event_handler(SWIGLUA_REF fn) {
        auto ref = std::make_shared<swiglua_ref>(fn);
        self->set_event_handler([ref] ( ranger::event::tcp_connection& self,
                                        ranger::event::tcp_connection::event_code what) {
            ref->get();
            SWIG_NewPointerObj(ref->L(), &self, SWIGTYPE_p_ranger__event__tcp_connection, 0);
            lua_pushnumber(ref->L(), static_cast<lua_Number>(what));
            lua_call(ref->L(), 2, 0);
        });
    }
}
