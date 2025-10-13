#ifndef KILN_PRINT_HPP
#define KILN_PRINT_HPP

# define KILN_PRINTER \
if (!message.empty() && !opt.no_messages && !opt.no_format) \
    printf("%s\n", message.c_str()); \
if (!script.empty() && !opt.no_scripts && !opt.no_format) \
    system(script.c_str());

# include "structs.hpp"

namespace Print
{
    void
    on_start(Hopt& opt, OutputFormat& outputFormat);

    void
    before_each_file(Hopt& opt, OutputFormat& outputFormat);

    void
    after_each_file(Hopt& opt, OutputFormat& outputFormat);

    void
    on_error(Hopt& opt, OutputFormat& outputFormat);

    void
    on_success(Hopt& opt, OutputFormat& outputFormat);

    void
    on_nothing_to_do(Hopt& opt, OutputFormat& outputFormat);

    void
    on_end(Hopt& opt, OutputFormat& outputFormat);
}

#endif
