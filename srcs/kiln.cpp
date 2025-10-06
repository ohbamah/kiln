#include "kiln.hpp"

std::map<std::string, Kiln::CmdAndDescription>
Kiln::default_commands = {
    { "base", { "echo base", "Compile all source files" } },
    { "watch", { "echo watch", "Watching files" } },
    { "clean", { "echo clean", "Delete all object files" } },
    { "oclean", { "echo oclean", "Delete output file (binary/library)" } },
    { "fclean", { "echo fclean", "Delete all object files and generated output (binary/library)" } },
    { "re", { "echo re", "Delete all object files and generated output (binary/library) and then compile" } }
};

void*
Kiln::FindOrCreateShellCommand(void* arg)
{
    Rule*   props = (Rule*)arg;

    if (props->cmd.empty() && Kiln::default_commands.contains(props->rulename))
        system(Kiln::default_commands[props->rulename].first);
    else
        system(props->cmd.c_str());
    return NULL;
}

void
Kiln::construct_final_command(FinalCommand& finalCommand)
{
    std::string& commandStringified = finalCommand.commandStringified;
    std::string objectFile;
    std::string dependenciesFile;
    std::string objectCompilationCommand;
    std::string finalFile = finalCommand.output.path + "/" + finalCommand.output.name;

    if (access(finalFile.c_str(), F_OK) != 0)
        finalCommand.isFinalFileExists = false;
    if (finalCommand.objects.active)
    {
        for (unsigned long i = 0 ; i < finalCommand.sourceFiles.size() ; ++i)
        {
            // use cache memory (static vars)
            objectFile = Utils::change_path_filename(Utils::remove_extension_in_filename(finalCommand.sourceFiles[i]), finalCommand.objects.path, !finalCommand.objects.relative) + finalCommand.objects.suffix + "." + finalCommand.objects.extension;
            dependenciesFile = Utils::change_path_filename(Utils::remove_extension_in_filename(finalCommand.sourceFiles[i]), finalCommand.dependencies.path, !finalCommand.dependencies.relative) + finalCommand.dependencies.suffix + "." + finalCommand.dependencies.extension;

            if (!finalCommand.isFinalFileExists ||
                    (Utils::compare_source_timestamp(finalCommand.sourceFiles[i], objectFile) == RECOMPILE ||
                    (finalCommand.dependencies.active && Utils::compare_dependencies_timestamps(objectFile, dependenciesFile) == RECOMPILE)))
            {
                char*   path = strdup(objectFile.c_str());
                objectCompilationCommand += "mkdir -p " + std::string(Utils::dirname(path)) + "\n";
                free(path);
                if (finalCommand.dependencies.active)
                {
                    path = strdup(dependenciesFile.c_str());
                    objectCompilationCommand += "mkdir -p " + std::string(Utils::dirname(path)) + "\n";
                    free(path);
                }
                objectCompilationCommand += finalCommand.compiler;
                objectCompilationCommand += finalCommand.options;
                objectCompilationCommand += finalCommand.includes;
                objectCompilationCommand += "-c " + finalCommand.sourceFiles[i] + " ";
                finalCommand.objectFiles.emplace_back(objectFile);
                objectCompilationCommand += "-o " + objectFile;
                if (finalCommand.dependencies.active)
                {
                    objectCompilationCommand += " -MMD -MP";
                    objectCompilationCommand += " -MF " + dependenciesFile;
                }
                // commandStringified += "\n";
                finalCommand.objectCompilationCommands.emplace_back(objectCompilationCommand);
            }
            else
                finalCommand.objectCompilationCommands.emplace_back("");
        }
    }

    finalCommand.filesToCompile = &finalCommand.sourceFiles;
    std::vector<std::string>*   files = finalCommand.filesToCompile;

    if (finalCommand.objects.active)
        files = &finalCommand.objectFiles;

    if (files->size() > 0)
    {
        char*   path = strdup(finalFile.c_str());
        commandStringified += "mkdir -p " + std::string(Utils::dirname(path)) + "\n";
        free(path);
        commandStringified += finalCommand.compiler;
        commandStringified += finalCommand.links_paths;
        commandStringified += finalCommand.links;
        for (unsigned long i = 0 ; i < files->size() ; ++i)
            commandStringified += " " + (*files)[i];
        commandStringified += " -o " + finalFile;
    }
    // std::cout << commandStringified << std::endl;
}

void
Kiln::print_final_commands(const FinalCommand& finalCommand)
{
    for (unsigned long i = 0 ; i < finalCommand.objectCompilationCommands.size() ; ++i)
        std::cout << finalCommand.objectCompilationCommands[i] << std::endl;
    std::cout << finalCommand.commandStringified;
    if (!finalCommand.commandStringified.empty())
        std::cout << std::endl;
}

void
Kiln::execute_final_commands(FinalCommand& finalCommand)
{
    bool            error = false;
    std::string*    messages_before = &finalCommand.outputFormat.messages.for_each_file.before;
    std::string*    messages_after = &finalCommand.outputFormat.messages.for_each_file.after;
    std::string*    messages_on_start = &finalCommand.outputFormat.messages.on_start;
    std::string*    messages_on_end = &finalCommand.outputFormat.messages.on_end;
    std::string*    messages_on_error = &finalCommand.outputFormat.messages.on_error;
    std::string*    messages_on_success = &finalCommand.outputFormat.messages.on_success;
    std::string*    messages_on_nothing_to_do = &finalCommand.outputFormat.messages.on_nothing_to_do;

    std::string*    scripts_before = &finalCommand.outputFormat.scripts.for_each_file.before;
    std::string*    scripts_after = &finalCommand.outputFormat.scripts.for_each_file.after;
    std::string*    scripts_on_start = &finalCommand.outputFormat.scripts.on_start;
    std::string*    scripts_on_end = &finalCommand.outputFormat.scripts.on_end;
    std::string*    scripts_on_error = &finalCommand.outputFormat.scripts.on_error;
    std::string*    scripts_on_success = &finalCommand.outputFormat.scripts.on_success;
    std::string*    scripts_on_nothing_to_do = &finalCommand.outputFormat.scripts.on_nothing_to_do;

    std::cout << "TEST: " << *scripts_on_start << std::endl;

    if (finalCommand.objectCompilationCommands.size() > 0)
    {
        if (!messages_on_start->empty())
            printf(std::string(messages_on_start->c_str() + std::string("\n")).c_str(), finalCommand.output.name.c_str());
        if (!scripts_on_start->empty())
            system((std::string("sh -c \"") + *scripts_on_start + "\"").c_str());
        for (unsigned long i = 0 ; i < finalCommand.objectCompilationCommands.size() ; ++i)
        {
            if (!finalCommand.objectCompilationCommands.empty())
            {
                // Put Hopt verbose option here

                // before
                if (!messages_before->empty())
                    printf(std::string(messages_before->c_str() + std::string("\n")).c_str(), finalCommand.sourceFiles[i].c_str());
                if (!scripts_before->empty())
                    system((std::string("sh -c \"") + *scripts_before + "\"").c_str());
                // real command
                if (system(finalCommand.objectCompilationCommands[i].c_str()) != 0)
                    error = true;
                // after
                if (!scripts_after->empty())
                    system((std::string("sh -c \"") + *scripts_after + "\"").c_str());
                if (!messages_after->empty())
                    printf(std::string(messages_after->c_str() + std::string("\n")).c_str(), finalCommand.sourceFiles[i].c_str());
            }
        }
        system(finalCommand.commandStringified.c_str());
        if (error)
        {
            if (!messages_on_error->empty())
                printf(std::string(messages_on_error->c_str() + std::string("\n")).c_str(), finalCommand.output.name.c_str());
            if (!scripts_on_error->empty())
                system((std::string("sh -c \"") + *scripts_on_error + "\"").c_str());
        }
        else if (!error)
        {
            if (!messages_on_success->empty())
                printf(std::string(messages_on_success->c_str() + std::string("\n")).c_str(), finalCommand.output.name.c_str());
            if (!scripts_on_success->empty())
                system((std::string("sh -c \"") + *scripts_on_success + "\"").c_str());
        }
        if (!scripts_on_end->empty())
            system((std::string("sh -c \"") + *scripts_on_end + "\"").c_str());
        if (!messages_on_end->empty())
            printf(std::string(messages_on_end->c_str() + std::string("\n")).c_str(), finalCommand.output.name.c_str());
    }
    else
    {
        if (!messages_on_nothing_to_do->empty())
            printf(std::string(messages_on_nothing_to_do->c_str() + std::string("\n")).c_str(), finalCommand.output.name.c_str());
        if (!scripts_on_nothing_to_do->empty())
            system((std::string("sh -c \"") + *scripts_on_nothing_to_do + "\"").c_str());
    }
}
