#include "generate.hpp"

void
Generate::options(std::string& finalOptions, Node& options)
{
    if (options)
    {
        if (options.kind() == Kinds::String)
            finalOptions = std::string(options) + " ";
        else if (options.kind() == Kinds::List)
        {
            for (unsigned long i = 0 ; i < options.size() ; ++i)
                if (options[i].kind() == Kinds::String)
                    finalOptions.append(std::string(options[i]) + " ");
        }
    }
}

void
Generate::includes_paths(std::string& finalIncludes, Node& includes)
{
    if (includes && includes.kind() == Kinds::List)
        for (unsigned long i = 0 ; i < includes.size() ; ++i)
            finalIncludes += "-I" + std::string(includes[i]) + " ";
}

void
Generate::links_paths(std::string& finalLinksPathes, Node& links_paths)
{
    if (links_paths && links_paths.kind() == Kinds::List)
        for (unsigned long i = 0 ; i < links_paths.size() ; ++i)
            if (links_paths[i].kind() == Kinds::String)
                finalLinksPathes += "-L" + std::string(links_paths[i]) + " ";
}

void
Generate::links(std::string& finalLinks, Node& links)
{
    if (links && links.kind() == Kinds::List)
        for (unsigned long i = 0 ; i < links.size() ; ++i)
            if (links[i].kind() == Kinds::String)
                finalLinks += "-l:" + std::string(links[i]) + " ";
}
