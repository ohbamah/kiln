#include "fetch.hpp"

void
Fetch::source_files(std::__1::vector<std::__1::string>& sourceFiles, Node& sources)
{
    // sources.mustBe(Kinds::List);
    if (sources["raw_paths"])
    {
        Node&   raw_paths = sources["raw_paths"];

        for (unsigned long i = 0 ; i < raw_paths.size() ; ++i)
            sourceFiles.emplace_back(std::string(raw_paths[i]) + " ");
     }
    if (sources["paths"])
    {
        Node&   paths = sources["paths"];
        std::string prefix;
        std::string suffix;

        if (sources["prefix"])
            prefix = std::string(sources["prefix"]);
        if (sources["suffix"])
            suffix = std::string(sources["suffix"]);
    
        for (unsigned long i = 0 ; i < paths.size() ; ++i)
            sourceFiles.emplace_back(prefix + std::string(paths[i]) + suffix);
    }
}

void
Fetch::compiler(std::string& finalCompiler, Node& compiler)
{
    if (compiler && compiler.kind() == Kinds::String)
        finalCompiler = std::string(compiler) + " ";
}

void
Fetch::objects_info(ObjectInfo& objectInfo, Node& object)
{
    if (object)
    {
        if (object["suffix"] && object["suffix"].kind() == Kinds::String)
            objectInfo.suffix = std::string(object["suffix"]);
        if (object["relative"].kind() == Kinds::Boolean)
            objectInfo.relative = (bool)object["relative"];
        else
            objectInfo.relative = true;
        if (object["path"] && object["path"].kind() == Kinds::String)
            objectInfo.path = std::string(object["path"]);
        else
            objectInfo.path = ".objs";
        if (object["extension"] && object["extension"].kind() == Kinds::String)
            objectInfo.extension = std::string(object["extension"]);
        else
            objectInfo.extension = "o";
        objectInfo.active = true;
    }
    else
        objectInfo.active = false;
}

void
Fetch::dependencies_info(Dependencies& dependencies, Node& headers)
{
    if (headers)
    {
        if (headers["suffix"] && headers["suffix"].kind() == Kinds::String)
            dependencies.suffix = std::string(headers["suffix"]);
        if (headers["relative"].kind() == Kinds::Boolean)
            dependencies.relative = (bool)headers["relative"];
        else
            dependencies.relative = true;
        if (headers["path"] && headers["path"].kind() == Kinds::String)
            dependencies.path = std::string(headers["path"]);
        else
            dependencies.path = ".objs";
        if (headers["extension"] && headers["extension"].kind() == Kinds::String)
            dependencies.extension = std::string(headers["extension"]);
        else
            dependencies.extension = "d";
        dependencies.active = true;
    }
    else
        dependencies.active = false;
}

void
Fetch::output(Output& finalOutput, Node& output)
{
    if (output)
    {
        if (output["kind"] && output["kind"].kind() == Kinds::String)
        {
            if (std::string(output["kind"]) == "bin")
                finalOutput.type = OutputType::Binary;
            else if (std::string(output["kind"]) == "lib")
                finalOutput.type = OutputType::Library;
            else
                finalOutput.type = OutputType::Binary;
        }
        if (output["path"] && output["path"].kind() == Kinds::String)
            finalOutput.path = std::string(output["path"]);
        else
            finalOutput.path = ".";
        if (output["name"] && output["name"].kind() == Kinds::String)
            finalOutput.name = std::string(output["name"]);
        else
            finalOutput.name = "a.out";
    }
    else
    {
        finalOutput.type = OutputType::Binary;
        finalOutput.name = "a.out";
        finalOutput.path = ".";
    }
}

void
parse_conditional_format(ConditionalFormat& conditionalFormat, Node& format)
{
    if (format)
    {
        if (format["on_success"])
            conditionalFormat.on_success = std::string(format["on_success"]);
        if (format["on_error"])
            conditionalFormat.on_error = std::string(format["on_error"]);
        if (format["on_start"])
            conditionalFormat.on_start = std::string(format["on_start"]);
        if (format["on_end"])
            conditionalFormat.on_end = std::string(format["on_end"]);
        if (format["on_nothing_to_do"])
            conditionalFormat.on_nothing_to_do = std::string(format["on_nothing_to_do"]);
        if (format["for_each_file"])
        {
            Node&   for_each_file = format["for_each_file"];

            if (for_each_file.kind() == Kinds::Map)
            {
                if (for_each_file["before"])
                    conditionalFormat.for_each_file.before = std::string(for_each_file["before"]);
                if (for_each_file["after"])
                    conditionalFormat.for_each_file.after = std::string(for_each_file["after"]);
            }
            else
                conditionalFormat.for_each_file.after = std::string(for_each_file);
        }
    }
}

void
Fetch::output_format(OutputFormat& outputFormat, Node& output_format)
{
    if (output_format)
    {
        if (output_format["colors"])
            outputFormat.colors = true;
        if (output_format["show_commands"])
            outputFormat.showCommands = true;
        else
            outputFormat.showCommands = false;
        if (output_format["show_stats"])
            outputFormat.showStats = true;
        else
            outputFormat.showStats = false;
        parse_conditional_format(outputFormat.messages, output_format["messages"]);
        parse_conditional_format(outputFormat.scripts, output_format["scripts"]);
    }
    else
    {
        outputFormat.colors = true;
        outputFormat.showCommands = true;
        outputFormat.showStats = false;
    }
}
