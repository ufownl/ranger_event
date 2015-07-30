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

#ifndef RANGER_EVENT_ENDPOINT_HPP
#define RANGER_EVENT_ENDPOINT_HPP

#ifndef SWIG
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#endif	// !SWIG

namespace ranger { namespace event {

class endpoint {
public:
#ifndef SWIG
	endpoint();
#endif	// !SWIG
	explicit endpoint(int port);
	endpoint(const char* ip_addr, int port);
#ifndef SWIG
	endpoint(const std::string& ip_addr, int port);
#endif	// !SWIG

	std::string addr() const {
		return inet_ntoa(m_sin.sin_addr);
	}

	int port() const {
		return ntohs(m_sin.sin_port);
	}

#ifndef SWIG
#ifdef RANGER_INTERNAL
public:
#else
private:
#endif	// RANGER_INTERNAL
	explicit endpoint(const sockaddr_in& sin)
		: m_sin(sin) {
		// nop
	}

	const sockaddr_in& _sockaddr_in() const {
		return m_sin;
	}

private:
	sockaddr_in m_sin;
#endif	// !SWIG
};

#ifndef SWIG
inline std::ostream& operator << (std::ostream& out, const endpoint& ep) {
	return out << ep.addr() << ":" << ep.port();
}
#endif	// !SWIG

} }

#endif	// RANGER_EVENT_ENDPOINT_HPP
