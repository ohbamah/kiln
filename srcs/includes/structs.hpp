#ifndef KILN_STRUCTS_HPP
#define KILN_STRUCTS_HPP

# define RECOMPILE      true
# define NOT_RECOMPILE  false

# include <string>
# include <vector>

struct Hopt
{
    char**      av;
    int         options_count;

    char        no_scripts;
    char        no_messages;
    char        no_format;
    char        is_watching;
    char        dry_run;
    char        verbose;
    const char* config_filename;
};

struct Rule
{
    Rule(const std::string& rulename, const std::string& run) : rulename(rulename), run(run)
    {}
    ~Rule()
    {}

    std::string rulename;
    std::string run;
    Hopt*       opt;
};

enum class OutputType
{
    Binary,
    Library
};

struct Output
{
    std::string                 name;
    std::string                 path;
    OutputType                  type;
};

struct FormatForEachFile
{
    std::string before;
    std::string after;
};

struct ConditionalFormat
{
    std::string         on_start;
    std::string         on_end;
    std::string         on_error;
    std::string         on_success;
    std::string         on_nothing_to_do;
    FormatForEachFile   for_each_file;
};

struct OutputFormat
{
    ConditionalFormat   scripts;
    ConditionalFormat   messages;
    bool                showStats;
    bool                showCommands;
    bool                colors;
};

struct ObjectInfo
{
    std::string suffix;
    std::string path;
    std::string extension;
    bool        relative;
    bool        active;
};

struct Dependencies
{
    std::string suffix;
    std::string path;
    std::string extension;
    bool        relative;
    bool        active;
};

struct Commands
{
    std::string                 mkdirP;
    std::vector<std::string>    objectCompilation;
    std::string                 outputCompilation;
    std::string                 finalExec;
};

struct KilnData
{
    std::string                 compiler;
    std::string                 options;
    std::string                 runtime_options;
    std::string                 links;
    std::string                 includes;
    std::string                 links_paths;
    std::vector<std::string>    sourceFiles;
    std::vector<std::string>    objectFiles;
    std::vector<std::string>    headerDependenciesFiles;
    // std::vector<std::string>    objectCompilationCommands;
    std::vector<std::string>*   filesToCompile;
    // std::string                 commandStringified;
    // std::string                 commandMkdirP;
    Commands                    commands;
    Output                      output;
    OutputFormat                outputFormat;
    ObjectInfo                  objects;
    Dependencies                dependencies;
    bool                        isFinalFileExists = true;

    void
    clear(void)
    {
        this->compiler.clear();
        this->options.clear();
        this->runtime_options.clear();
        this->links.clear();
        this->includes.clear();
        this->links_paths.clear();
        this->sourceFiles.clear();
        this->objectFiles.clear();
        this->headerDependenciesFiles.clear();
        this->commands.mkdirP.clear();
        this->commands.finalExec.clear();
        this->commands.objectCompilation.clear();
        this->commands.outputCompilation.clear();
    }
};

#endif
