#include "kiln.hpp"

std::map<std::string, std::string>
default_commands = {
    { "compile", "echo compile" },
    { "watch", "echo watch" },
    { "clean", "echo clean" },
    { "fclean", "echo fclean" },
    { "re", "echo re" },
    { "remove_binary", "echo remove_binary" },
    { "remove_objects", "echo remove_objects" }
};

void*
Kiln::FindOrCreateShellCommand(void* arg)
{
    char*   cmd = (char*)arg;

    if (Kiln::default_commands.contains(cmd))
        system(Kiln::default_commands[cmd].data());
    else
        system(cmd);
    return NULL;
}
