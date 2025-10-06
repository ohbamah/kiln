#ifndef KILN_FETCH_HPP
#define KILN_FETCH_HPP

# include "kiln.hpp"
# include "structs.hpp"

namespace Fetch
{
    void
    source_files(std::__1::vector<std::__1::string>& sourceFiles, Node& sources);

    void
    compiler(std::string& finalCompiler, Node& compiler);

    void
    objects_info(ObjectInfo& objectInfo, Node& object);

    void
    dependencies_info(Dependencies& dependencies, Node& headers);

    void
    output(Output& finalOutput, Node& output);

    void
    output_format(OutputFormat& outputFormat, Node& output_format);
}

#endif
