#ifndef KILN_STRUCTS_HPP
#define KILN_STRUCTS_HPP

# define RECOMPILE      true
# define NOT_RECOMPILE  false

# include <string>
# include <vector>

struct Rule
{
    Rule(const std::string& rulename, const std::string& cmd) : rulename(rulename), cmd(cmd)
    {}
    ~Rule()
    {}

    std::string rulename;
    std::string cmd;
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

struct FinalCommand
{
    std::string                 compiler;
    std::string                 options;
    std::string                 links;
    std::string                 includes;
    std::string                 links_paths;
    std::vector<std::string>    sourceFiles;
    std::vector<std::string>    objectFiles;
    std::vector<std::string>    headerDependenciesFiles;
    std::vector<std::string>    objectCompilationCommands;
    std::vector<std::string>*   filesToCompile;
    std::string                 commandStringified;
    Output                      output;
    OutputFormat                outputFormat;
    ObjectInfo                  objects;
    Dependencies                dependencies;
    bool                        isFinalFileExists = true;
};

struct Hopt
{
    char    dry_run;
    char    verbose;
};

#endif
