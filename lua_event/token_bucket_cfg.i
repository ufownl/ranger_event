%{
#include <event/token_bucket_cfg.hpp>
%}

%ignore ranger::event::token_bucket_cfg;
%include "event/token_bucket_cfg.hpp"

%shared_ptr(ranger::event::token_bucket_cfg);

%{
#include <chrono>

std::shared_ptr<const ranger::event::token_bucket_cfg>
create_token_bucket_cfg(    size_t read_rate, size_t read_burst,
                            size_t write_rate, size_t write_burst,
                            double sec = 0.0) {
    return ranger::event::token_bucket_cfg::create(read_rate, read_burst,
                                                   write_rate, write_burst,
                                                   std::chrono::duration<double>(sec));
}
%}

std::shared_ptr<const ranger::event::token_bucket_cfg>
create_token_bucket_cfg(    size_t read_rate, size_t read_burst,
                            size_t write_rate, size_t write_burst,
                            double sec = 0.0);
