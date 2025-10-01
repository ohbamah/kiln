#include "main.hpp"



void
handle_hopt_error()
{
    char*   hopterror = hopt_strerror();
    printf("%s\n", hopterror);
    free(hopterror);
    hopt_free();
    exit(1);
}

void
generate_command(const char* rule, const char* cmd)
{
    (void)rule;
    (void)cmd;
}

char*
create_long_option_rule(const std::string& rule)
{
    return strdup(("-" + rule).data());
}

void
execcmd(const std::string& cmd)
{
    int pid = fork();
    if (!pid)
    {
        std::vector<std::string> args;
        std::string current;
        
        // Parse la commande en mots
        for (unsigned long i = 0; i < cmd.size(); ++i) {
            if (cmd[i] == ' ' && !current.empty()) {
                args.push_back(current);
                current.clear();
            } else if (cmd[i] != ' ') {
                current.push_back(cmd[i]);
            }
        }
        if (!current.empty())
            args.push_back(current);

        // Convertir en char**
        char** argv = new char*[args.size() + 1];
        for (unsigned long i = 0; i < args.size(); ++i) {
            argv[i] = strdup(args[i].c_str());
        }
        argv[args.size()] = NULL;

        execvp(argv[0], argv);
        
        for (unsigned long i = 0; i < args.size(); ++i)
            free(argv[i]);
        delete[] argv;
        exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
}

void
generate_rules_as_subcmd(Node& config)
{
    Node   rules;
    try { rules = config.require("rules"); } catch (...) { std::cout << "No rules\n"; return; }
    for (unsigned long i = 0 ; i < rules.size() ; ++i)
    {
        Node&   rule = rules[i];
        Node    cmd;

        try {
            cmd = rule.require("cmd");
        } catch (...) {}

        const std::string&  ruleName = rules.keyname(i);
        std::string         description;
        if (rule["description"])
            description = (std::string)rule["description"];
        hopt_subcmd(strdup(ruleName.data()), Kiln::FindOrCreateShellCommand, (void*)std::string(rules[i]).c_str(), NULL)
        {
            hopt_help_option((char*)"h=-help", 1, 0);
            hopt_program_description(strdup(description.data()));
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
    char*   config_filename = DEFAULT_CONFIG_FILENAME;

    hopt_program_description((char*)"Kiln is a tool to compile C/Clay files with a yml config file.");
    hopt_group((char*)"\033[1mKILN OPTIONS:\033[0m");
    hopt_add_option((char*)"c=-config", 1, 0, &config_filename, "Path of the config file");

        // For only the KILN's DEFAULT options
    hopt_allow_undef();
    int c = hopt(ac, av);
    if (c == -1)
        handle_hopt_error();
    hopt_reset(); // Reset `allow_undef`

    // Try to open the YAML file
    Node    yml;
    try {
        yml = Node(config_filename);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    hopt_help_option((char*)"h=-help", true, HOPT_UNDEFINED | HOPT_MISSOARGC | HOPT_REDEFINED | HOPT_BADSORDER | HOPT_BADTYPE_NUM);
    try {
        yml.required("clay");
        handle_subcmd(yml);
    } catch(const Node::KeyIsRequired& e) {
        printf("Namespace `clay:` is required for a clay YAML config file.\n");
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    hopt_end_on_arg(); // Because options/cmds are sorts with the first call to `hopt`
    c = hopt(ac, av);
    if (c == -1)
        handle_hopt_error();
    hopt_free();
    return 0;
}

int main(int ac, char** av)
{
    return kiln(ac, av);
}
