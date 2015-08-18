// Copyright (c) 2015, RangerUFO
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// 
// * Neither the name of ranger_event nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

%{
#include <ranger/event/tcp_connection.hpp>
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

%include "ranger/event/tcp_connection.hpp"

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
