#ifndef KILN_UTILS_HPP
#define KILN_UTILS_HPP

# define CLOSE_FILE_AND_RETURN(file) { fclose(file); return NOT_RECOMPILE; }

# include <string>
# include <sys/stat.h>
# include <sys/mman.h>
# include "structs.hpp"

namespace Utils
{
    std::string
    change_path_filename(const std::string& filename, const std::string& new_path, bool is_relative);

    std::string
    remove_extension_in_filename(const std::string& filename);

    bool
    compare_source_timestamp(const std::string& sourceFilename, const std::string& objectFilename);

    bool
    compare_dependencies_timestamps(const std::string& objectFilename, const std::string& dependenciesFilename);

    unsigned long
    get_file_size(FILE* file);

    bool
    parse_dependencies_file(std::vector<std::string>& dependenceFiles, const std::string& dependenciesFilename);

    char*
    dirname(char* path);
}

#endif
