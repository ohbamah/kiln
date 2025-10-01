#ifndef KILN_HPP
#define KILN_HPP

# include "nodes.hpp"
# include <sys/wait.h>
# include <cstdlib>
# include <unistd.h>
# include <map>
# include <string>

namespace Kiln
{
    void
    AddToEnv(const Node& env);

    void*
    FindOrCreateShellCommand(void* arg);

    static
    std::map<std::string, std::string>
    default_commands;
};

#endif
