%{
#include <event/dispatcher.hpp>
%}

%include "event/dispatcher.hpp"

%extend ranger::event::dispatcher {
    void exit(double sec) {
        self->exit(std::chrono::duration<double>(sec));
    }
}
