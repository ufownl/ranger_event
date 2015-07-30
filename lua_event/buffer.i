%{
#include <event/buffer.hpp>
%}

%include "carrays.i"
%array_class(unsigned char, byte_array);

%include "cdata.i"

%{
#include <string.h>

SWIGCDATA str2cdata(char* str) {
    return {str, static_cast<int>(strlen(str)) + 1};
}
%}

SWIGCDATA str2cdata(char* str);

%include "std_string.i"
%include "event/buffer.hpp"

%extend ranger::event::buffer {
    bool append(SWIGCDATA cdata) {
        return self->append(cdata.data, cdata.len);
    }

    int remove(SWIGCDATA cdata) {
        return self->remove(cdata.data, cdata.len);
    }

    ssize_t copyout(SWIGCDATA cdata) {
        return self->copyout(cdata.data, cdata.len);
    }

    size_t search(SWIGCDATA cdata, size_t pos = 0) {
        return self->search(cdata.data, cdata.len, pos);
    }
}
