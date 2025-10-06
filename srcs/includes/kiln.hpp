#ifndef KILN_HPP
#define KILN_HPP

# include "nodes.hpp"
# include "structs.hpp"
# include "utils.hpp"
# include <sys/wait.h>
# include <cstdlib>
# include <unistd.h>
# include <map>
# include <vector>
# include <string>

namespace Kiln
{
    typedef std::pair<const char*, const char*> CmdAndDescription;

    void
    AddToEnv(const Node& env);

    void*
    FindOrCreateShellCommand(void* arg);

    void
    construct_final_command(FinalCommand& finalCommand);

    void
    print_final_commands(const FinalCommand& finalCommand);

    void
    execute_final_commands(FinalCommand& finalCommand);

    extern
    std::map<std::string, Kiln::CmdAndDescription>
    default_commands;
};

#endif
