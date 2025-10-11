#ifndef KILN_PRINT_HPP
#define KILN_PRINT_HPP

# define KILN_PRINTER \
if (!message.empty()) \
    printf("%s\n", message.c_str()); \
if (!script.empty()) \
    system(script.c_str());

# include "structs.hpp"

namespace Print
{
    void
    on_start(OutputFormat& outputFormat);

    void
    before_each_file(OutputFormat& outputFormat);

    void
    after_each_file(OutputFormat& outputFormat);

    void
    on_error(OutputFormat& outputFormat);

    void
    on_success(OutputFormat& outputFormat);

    void
    on_nothing_to_do(OutputFormat& outputFormat);

    void
    on_end(OutputFormat& outputFormat);
}

#endif
