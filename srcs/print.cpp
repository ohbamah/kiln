#include "print.hpp"

void
Print::on_start(Hopt& opt, OutputFormat& outputFormat)
{
    std::string&    message = outputFormat.messages.on_start;
    std::string&    script = outputFormat.scripts.on_start;

    KILN_PRINTER
}

void
Print::before_each_file(Hopt& opt, OutputFormat& outputFormat)
{
    std::string&    message = outputFormat.messages.for_each_file.before;
    std::string&    script = outputFormat.scripts.for_each_file.before;

    KILN_PRINTER
}

void
Print::after_each_file(Hopt& opt, OutputFormat& outputFormat)
{
    std::string&    message = outputFormat.messages.for_each_file.after;
    std::string&    script = outputFormat.scripts.for_each_file.after;

    KILN_PRINTER
}

void
Print::on_error(Hopt& opt, OutputFormat& outputFormat)
{
    std::string&    message = outputFormat.messages.on_error;
    std::string&    script = outputFormat.scripts.on_error;

    KILN_PRINTER
}

void
Print::on_success(Hopt& opt, OutputFormat& outputFormat)
{
    std::string&    message = outputFormat.messages.on_success;
    std::string&    script = outputFormat.scripts.on_success;

    KILN_PRINTER
}

void
Print::on_nothing_to_do(Hopt& opt, OutputFormat& outputFormat)
{
    std::string&    message = outputFormat.messages.on_nothing_to_do;
    std::string&    script = outputFormat.scripts.on_nothing_to_do;

    KILN_PRINTER
}

void
Print::on_end(Hopt& opt, OutputFormat& outputFormat)
{
    std::string&    message = outputFormat.messages.on_end;
    std::string&    script = outputFormat.scripts.on_end;

    KILN_PRINTER
}
