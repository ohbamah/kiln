#ifndef KILN_GENERATE_HPP
#define KILN_GENERATE_HPP

# include "kiln.hpp"
# include "structs.hpp"

namespace Generate
{
    void
    options(std::string& finalOptions, Node& options);

    void
    includes_paths(std::string& finalIncludes, Node& includes);

    void
    links_paths(std::string& finalLinksPathes, Node& links_paths);

    void
    links(std::string& finalLinks, Node& links);
}

#endif
