%{
#include <ranger/event/dispatcher.hpp>
%}

%include "ranger/event/dispatcher.hpp"

%extend ranger::event::dispatcher {
    void exit(double sec) {
        self->exit(std::chrono::duration<double>(sec));
    }
}
