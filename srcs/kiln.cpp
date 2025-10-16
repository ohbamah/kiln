#include "kiln.hpp"

std::map<std::string, Kiln::TRulesAndDescription>
Kiln::rules_map = {
    { "clean",  { KR_CLEAN } },
    { "oclean", { KR_OCLEAN } },
    { "fclean", { KR_FCLEAN } },
    { "re",     { KR_RE } }
};

void*
Kiln::find_or_create_shell_subcommand(void* arg)
{
    Rule*       props = (Rule*)arg;
    std::string cmd;

    if (!hopt_help_has_been_called_v)
    {
        if (props->run.empty() && Kiln::rules_map.contains(props->rulename))
            cmd = Kiln::rules_map[props->rulename].first;
        else
            cmd = props->run;
        Kiln::ParseRuleCallsInRules(*props->opt, cmd);

        if (props->opt->verbose)
            printf("%s\n", cmd.data());
        if (!props->opt->dry_run)
            system(cmd.data());
    }

    Kiln::at_least_one_subcommand_has_been_executed = true;
    return NULL;
}

void
Kiln::construct_output_command(KilnData& kilnData)
{
    std::string& commandStringified = kilnData.commands.outputCompilation;
    std::string sourceFile;
    std::string objectFile;
    std::string dependenciesFile;
    std::string objectCompilationCommand;
    std::string finalFile = kilnData.output.path + "/" + kilnData.output.name;

    if (access(finalFile.c_str(), F_OK) != 0)
        kilnData.isFinalFileExists = false;
    if (kilnData.objects.active)
    {
        for (unsigned long i = 0 ; i < kilnData.sourceFiles.size() ; ++i)
        {
            // use cache memory (static vars)
            sourceFile = kilnData.sourceFiles[i];
            objectFile = Utils::change_path_filename(Utils::remove_extension_in_filename(kilnData.sourceFiles[i]), kilnData.objects.path, !kilnData.objects.relative) + kilnData.objects.suffix + "." + kilnData.objects.extension;
            dependenciesFile = Utils::change_path_filename(Utils::remove_extension_in_filename(kilnData.sourceFiles[i]), kilnData.dependencies.path, !kilnData.dependencies.relative) + kilnData.dependencies.suffix + "." + kilnData.dependencies.extension;

                // recompile object file
            if (!kilnData.isFinalFileExists ||
                    (Utils::compare_source_timestamp(kilnData.sourceFiles[i], objectFile) == RECOMPILE ||
                    (kilnData.dependencies.active && Utils::compare_dependencies_timestamps(objectFile, dependenciesFile) == RECOMPILE)))
            {
                objectCompilationCommand.clear();
                objectCompilationCommand += kilnData.compiler;
                objectCompilationCommand += kilnData.options;
                objectCompilationCommand += kilnData.runtime_options;
                objectCompilationCommand += kilnData.includes;
                objectCompilationCommand += "-c " + kilnData.sourceFiles[i] + " ";
                kilnData.objectFiles.emplace_back(objectFile);
                objectCompilationCommand += "-o " + objectFile;
                if (kilnData.dependencies.active)
                {
                    objectCompilationCommand += " -MMD -MP";
                    objectCompilationCommand += " -MF " + dependenciesFile;
                }
                // commandStringified += "\n";
                kilnData.commands.objectCompilation.emplace_back(objectCompilationCommand);
            }
                // not recompile object file
            else
                kilnData.objectFiles.emplace_back(objectFile);
        }
    }

    kilnData.filesToCompile = &kilnData.sourceFiles;
    std::vector<std::string>*   files = kilnData.filesToCompile;

    if (kilnData.objects.active)
        files = &kilnData.objectFiles;

    if (files->size() > 0)
    {
        commandStringified += kilnData.compiler;
        commandStringified += kilnData.runtime_options;
        commandStringified += kilnData.links_paths;
        commandStringified += kilnData.links;
        for (unsigned long i = 0 ; i < files->size() ; ++i)
            commandStringified += " " + (*files)[i];
        commandStringified += " -o " + finalFile;
    }
}

void
Kiln::print_final_commands(const KilnData& kilnData)
{
    for (unsigned long i = 0 ; i < kilnData.commands.objectCompilation.size() ; ++i)
        std::cout << kilnData.commands.objectCompilation[i] << std::endl;
    std::cout << kilnData.commands.finalExec;
    if (!kilnData.commands.outputCompilation.empty())
        std::cout << std::endl;
}
 
int
Kiln::execute_final_commands(Hopt& opt, KilnData& kilnData)
{
    bool            error = false;
    unsigned long   compiled_file = 0;

    if (!kilnData.commands.mkdirP.empty())
        system(kilnData.commands.mkdirP.c_str());
    if (kilnData.commands.objectCompilation.size() > 0)
    {
        Print::on_start(opt, kilnData.outputFormat);
        for (unsigned long i = 0 ; i < kilnData.commands.objectCompilation.size() ; ++i)
        {
            if (!kilnData.commands.objectCompilation[i].empty())
            {
                if (i < kilnData.sourceFiles.size())
                    setenv("SOURCE_FILENAME", kilnData.sourceFiles[i].c_str(), true);
                if (i < kilnData.objectFiles.size())
                    setenv("OBJECT_FILENAME", kilnData.objectFiles[i].c_str(), true);

                Print::before_each_file(opt, kilnData.outputFormat);
                if (system(kilnData.commands.objectCompilation[i].c_str()) != 0)
                    error = true;
                Print::after_each_file(opt, kilnData.outputFormat);
                ++compiled_file;
            }
        }
        if (compiled_file > 0)
        {
            if (error)
                Print::on_error(opt, kilnData.outputFormat);
            else if (!error)
            {
                system(kilnData.commands.finalExec.c_str());
                Print::on_success(opt, kilnData.outputFormat);
            }
        }
        else
            Print::on_nothing_to_do(opt, kilnData.outputFormat);
    }
    else
        Print::on_nothing_to_do(opt, kilnData.outputFormat);
    Print::on_end(opt, kilnData.outputFormat);
    return error ? 1 : 0;
}

void
Kiln::create_mkdir_p_command(KilnData& kilnData)
{
    std::string dirname_output_name = Utils::dirname(std::string(kilnData.output.name).data());

    kilnData.commands.mkdirP = "mkdir -p ";
    kilnData.commands.mkdirP += kilnData.output.path + "/" + dirname_output_name + " ";
    kilnData.commands.mkdirP += kilnData.dependencies.path + " ";
    kilnData.commands.mkdirP += kilnData.objects.path + " ";
    kilnData.commands.mkdirP += "\n";
}

void
Kiln::create_final_command(KilnData& kilnData)
{
    kilnData.commands.finalExec = kilnData.commands.outputCompilation;
}

void
Kiln::ParseRuleCallsInRules(Hopt& opt, std::string& ruleCmd)
{
    std::size_t     start = 0;
    std::size_t     end = 0;
    std::size_t     pos = 0;
    std::string     ruleRef = "";

    while (ruleCmd[pos] && pos != std::string::npos)
    {
        pos = ruleCmd.find_first_of('$', pos);
        if (pos == std::string::npos)
            break;
        if (ruleCmd[++pos] == '(')
        {
            // logic
            // Gérer les quotes

            ++pos; // Skip the first '('
            start = pos;
            for ( ; ruleCmd[pos] && ruleCmd[pos] != ')' ; ++pos)
                ;
            if (!ruleCmd[pos])
                break ;
            end = pos;
            ruleRef = ruleCmd.substr(start, end - start);
            if (!Kiln::rules_map.contains(ruleRef) && ruleRef != "base")
            {
                ++pos;
                continue ;
            }
            else if (ruleRef == "base")
            {
                std::string prog;
                for (int j = 0 ; j < opt.options_count + 1 ; ++j)
                    prog += opt.av[j] + std::string(" ");
                ruleCmd.replace(ruleCmd.begin() + start - 2, ruleCmd.begin() + end + 1, prog);
            }
            else
                ruleCmd.replace(ruleCmd.begin() + start - 2, ruleCmd.begin() + end + 1, Kiln::rules_map[ruleRef].first);
        }
        else
            while (ruleCmd[pos] && ruleCmd[pos] != ')')
                ++pos;
        ++pos; // skip the last ')'
    }
}
