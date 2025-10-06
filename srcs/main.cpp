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
generate_rules_as_subcmd(Node& config)
{
    Node&   rules = config["rules"];

    if (rules)
    {
        for (unsigned long i = 0 ; i < rules.size() ; ++i)
        {
            Node&               rule = rules[i];
            std::string         cmd;
            std::string         description;
            const std::string&  ruleName = rules.keyname(i);

            if (rule["description"])
                description = (std::string)rule["description"];
            if (rule["cmd"])
                cmd = (std::string)rule["cmd"];

            Rule*   rule_props = new Rule(ruleName, cmd); // To `delete`
            hopt_subcmd(strdup(ruleName.c_str()), Kiln::FindOrCreateShellCommand, (void*)rule_props, NULL)
            {
                hopt_help_option((char*)"h=-help", 1, 0);
                if (description.empty() && Kiln::default_commands.contains(ruleName))
                    description = Kiln::default_commands[ruleName].second;
                hopt_program_description(strdup(description.c_str()));
            }
        }
    }
}

void
handle_env(Node& config)
{
    try {
        Node&   env = config.require("env");

        for (unsigned long i = 0 ; i < env.size() ; ++i)
            setenv(env.keyname(i).c_str(), std::string(env[i]).c_str(), true);
    } catch (...)
    {}
}

void
handle_subcmd(Node& config)
{
    hopt_group((char*)"\033[1mCONFIGURATION FILE RULES:\033[0m");
    generate_rules_as_subcmd(config);
}

void
handle_final_command(FinalCommand& finalCommand, Node& config)
{
    Fetch::source_files(finalCommand.sourceFiles, config.require("sources"));
    Fetch::compiler(finalCommand.compiler, config["compiler"]);
    Generate::options(finalCommand.options, config["options"]);
    Generate::includes_paths(finalCommand.includes, config["includes_paths"]);
    Generate::links_paths(finalCommand.links_paths, config["links_paths"]);
    Generate::links(finalCommand.links, config["links"]);
    Fetch::objects_info(finalCommand.objects, config["objects"]);
    Fetch::dependencies_info(finalCommand.dependencies, config["objects"]["headers"]);
    Fetch::output(finalCommand.output, config["output"]);
    Fetch::output_format(finalCommand.outputFormat, config["output_format"]);
    Kiln::construct_final_command(finalCommand);
}

int kiln(int ac, char** av)
{
    Hopt    opt = { 0 };
    char*   config_filename = DEFAULT_CONFIG_FILENAME;

    hopt_program_description((char*)"Kiln is a tool to compile C/Clay files with a yml config file.");
    hopt_group((char*)"\033[1mKILN OPTIONS:\033[0m");
    hopt_add_option((char*)"c=-config", 1, 0, &config_filename, "Path of the config file");
    hopt_add_option((char*)"d=-dry-run", 0, 0, &opt.dry_run, "Does not execute the final command");
    hopt_add_option((char*)"v=-verbose", 0, 0, &opt.verbose, "Print the final command before execution");

        // For only the KILN's DEFAULT options
    hopt_allow_undef();
    int c = hopt(ac, av);
    if (c == -1)
        handle_hopt_error();
    hopt_reset(); // Reset `allow_undef`

    // Try to open the YAML file
    Node            yml;
    FinalCommand    finalCommand;
    try {
        yml = Node(config_filename);
    } catch (const std::exception& e) {
        hopt_print_help_menu(NULL);
        std::cout << "\n" << e.what() << std::endl;
        return 1;
    }

    hopt_help_option((char*)"h=-help", true, HOPT_UNDEFINED | HOPT_MISSOARGC | HOPT_REDEFINED | HOPT_BADSORDER | HOPT_BADTYPE_NUM);
    try {
        yml.required("clay");
        handle_env(yml);
        handle_subcmd(yml);
        handle_final_command(finalCommand, yml);
    } catch(const Node::KeyIsRequired& e) {
        printf("Namespace `clay:` is required for a clay YAML config file.\n");
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    hopt_end_on_arg(); // Because options/cmds are sorts with the first call to `hopt`
    c = hopt(ac, av);
    if (c == -1)
        handle_hopt_error();

    if (hopt_help_has_been_called())
        return 0;

    if (opt.verbose)
        Kiln::print_final_commands(finalCommand);
    if (!opt.dry_run)
        Kiln::execute_final_commands(finalCommand);

    hopt_free();
    return 0;
}

int main(int ac, char** av)
{
    return kiln(ac, av);
}
