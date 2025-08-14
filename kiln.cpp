#include "kiln.hpp"
#include "nodes.hpp"

extern "C" {
    #include "hopt.h"
}

#include <stdio.h>
#include <map>
#include <string>

std::map<std::string, std::string>
default_commands = {
    { "compile", "# Compile LOL, not necessary to compile unless you want a specific rule" }
    { "watch", "./forge --watch {...}" },
    { "clean", "# remove all objects / object path" },
    { "fclean", "# `clean` + remove output file" },
    { "re", "# fclean + compilation" }
};

void
link_command_to_buffer(const char* rule, const char* cmd)
{

}

void
generate_rules_as_subcmd(Node& config)
{
    try { config.required("rules"); } catch (...) { return; }
    for (unsigned long i = 0 ; i < config.size() ; ++i)
    {
        Node&           rule = config[i];
        const
        std::string&    ruleName = rule.keyname();
        hopt_subcmd((char*)ruleName.data());
        if (rule.kind() == YAP_Kinds::List)
        {
            for (unsigned long j = 0 ; j < rule.size() ; ++j)
            {
                Node&   command = rule[j];
                if (command.kind() == YAP_Kinds::Map)
                    for (auto& it: default_commands)
                        if (command.keyname() == it.first)
                            link_command_to_buffer(it.first.data(), it.second.data());
            }
        }
    }
}

void
handle_subcmd(Node& config)
{
    hopt_group((char*)"\033[1mCONFIGURATION FILE RULES:\033[0m");
    generate_rules_as_subcmd(config);
}

int kiln(int ac, char** av)
{
    char*   config_filename = (char*)"./clay.yml";
    hopt_allow_redef(true);
    hopt_program_description((char*)"Kiln is a tool to compile C/Clay files with a special config file.");
    hopt_help_option((char*)"h=-help", true, HOPT_MISSOARGC | HOPT_REDEFINED | HOPT_BADSORDER | HOPT_BADTYPE_NUM | HOPT_UNDEFINED);
    hopt_group((char*)"\033[1mBASIC:\033[0m");
    hopt_add_option((char*)"c=-config", 1, 0, &config_filename, "Path of the clay configuration file");;

    // Handle different .yml file names
    Node yml(config_filename);

    try {
        yml.required("clay"); // equivalent to --> yml = yml.require("clay");
        handle_subcmd(yml);
    } catch (...) {
        printf("Namespace `clay:` is required for a clay YAML config file.\n");
    }
    int c = hopt(ac, av);
    if (c == -1)
    {
        char*   hopterror = hopt_strerror();
        printf("%s\n");
        free(hopterror);
        return 1;
    }
    hopt_free();
    return 0;
}

int main(int ac, char** av)
{
    return kiln(ac, av);
}
