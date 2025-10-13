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
generate_basic_program_options(Hopt& opt)
{
    hopt_group((char*)"\033[1mKILN OPTIONS:\033[0m");
    hopt_add_option((char*)"c=-config", 1, 0, &opt.config_filename, "Path of the config file");
    hopt_add_option((char*)"d=-dry-run", 0, 0, &opt.dry_run, "Does not execute the final command");
    hopt_add_option((char*)"v=-verbose", 0, 0, &opt.verbose, "Print the final command before execution");
    hopt_add_option((char*)"w=-watch", 0, 0, &opt.is_watching, "Watch source files & dependencies, and if one of them is modified, recompile all");
    hopt_add_option((char*)"s=-no-scripts", 0, 0, &opt.no_scripts, "Don't execute scripts of the config file");
    hopt_add_option((char*)"m=-no-messages", 0, 0, &opt.no_messages, "Don't print messages of the config file");
    hopt_add_option((char*)"n=-no-format", 0, 0, &opt.no_format, "Do nothing about formatted output");
}

void
generate_rules_as_subcmd(Hopt& opt, Node& config)
{
    Node&   rules = config["rules"];

    if (rules)
    {
        for (unsigned long i = 0 ; i < rules.size() ; ++i)
        {
            Node&               rule = rules[i];
            std::string         run;
            std::string         description;
            const std::string&  ruleName = rules.keyname(i);

            if (rule["description"])
                description = (std::string)rule["description"];
            if (rule["run"])
                run = (std::string)rule["run"];

            Rule*   rule_props = new Rule(ruleName, run); // To `delete`
            rule_props->opt = &opt;
            hopt_subcmd(strdup(ruleName.c_str()), Kiln::find_or_create_shell_subcommand, (void*)rule_props, NULL)
            {
                hopt_help_option((char*)"h=-help", 1, 0);
                if (description.empty() && Kiln::rules_map.contains(ruleName))
                    description = Kiln::rules_map[ruleName].second;
                hopt_program_description(strdup(description.c_str()));
                generate_basic_program_options(opt);
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
handle_subcmd(Hopt& opt, Node& config)
{
    hopt_group((char*)"\033[1mCONFIGURATION FILE RULES:\033[0m");
    generate_rules_as_subcmd(opt, config);
}

void
handle_final_command(KilnData& kilnData, Node& config)
{
    Fetch::source_files(kilnData.sourceFiles, config.require("sources"));
    Fetch::compiler(kilnData.compiler, config["compiler"]);
    Generate::options(kilnData.options, config["options"]);
    Generate::options(kilnData.runtime_options, config["runtime_options"]);
    Generate::includes_paths(kilnData.includes, config["includes_paths"]);
    Generate::links_paths(kilnData.links_paths, config["links_paths"]);
    Generate::links(kilnData.links, config["links"]);
    Fetch::objects_info(kilnData.objects, config["objects"]);
    Fetch::dependencies_info(kilnData.dependencies, config["objects"]["headers"]);
    Fetch::output(kilnData.output, config["output"]);
    Fetch::output_format(kilnData.outputFormat, config["output_format"]);
    Kiln::create_mkdir_p_command(kilnData);
    Kiln::construct_output_command(kilnData);
    Kiln::create_final_command(kilnData);

    setenv("OUTPUT_PATH", kilnData.output.path.c_str(), true);
    setenv("OUTPUT_TYPE", kilnData.output.type == OutputType::Binary ? "binary" : "library", true);
    setenv("OUTPUT_FILENAME", kilnData.output.name.c_str(), true);
    setenv("OBJECTS_PATH", kilnData.objects.path.c_str(), true);
}

void
base_rule(Hopt& opt, KilnData& kilnData)
{
    if (opt.verbose)
        Kiln::print_final_commands(kilnData);
    if (!opt.dry_run)
        Kiln::execute_final_commands(opt, kilnData);
}

int kiln(int ac, char** av)
{
    Hopt    opt = {
        .config_filename = DEFAULT_CONFIG_FILENAME
    };

    hopt_program_description((char*)"Kiln is a tool to compile C/Clay files with a yml config file.");
    generate_basic_program_options(opt);

        // For only the KILN's DEFAULT options
    hopt_allow_undef();
    int c = hopt(ac, av);
    if (c == -1)
        handle_hopt_error();
    hopt_reset(); // Reset `allow_undef`

    // Try to open the YAML file
    Node            yml;
    KilnData    kilnData;
    try {
        yml = Node(opt.config_filename);
    } catch (const std::exception& e) {
        hopt_print_help_menu(NULL);
        std::cout << "\n" << e.what() << std::endl;
        return 1;
    }

    hopt_help_option((char*)"h=-help", true, HOPT_UNDEFINED | HOPT_MISSOARGC | HOPT_REDEFINED | HOPT_BADSORDER | HOPT_BADTYPE_NUM);
    try {
        yml.required("kiln");
        handle_env(yml);
        handle_subcmd(opt, yml);
        handle_final_command(kilnData, yml);
    } catch(const Node::KeyIsRequired& e) {
        printf("Namespace `kiln:` is required for a kiln YAML config file.\n");
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    hopt_end_on_arg(); // Because options/cmds are sorts with the first call to `hopt`
    c = hopt(ac, av);
    if (c == -1)
        handle_hopt_error();

    if (hopt_help_has_been_called())
        return 0;

    if (!Kiln::at_least_one_subcommand_has_been_executed)
        base_rule(opt, kilnData);

    hopt_free();

    if (opt.is_watching)
        Watch::Call(yml, opt, kilnData);

    return 0;
}

int main(int ac, char** av)
{
    return kiln(ac, av);
}
