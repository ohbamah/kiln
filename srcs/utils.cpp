#include "utils.hpp"

std::string
Utils::change_path_filename(const std::string& filename, const std::string& new_path, bool is_relative)
{
    std::string ret = filename;

    if (is_relative)
    {
        std::size_t pos = ret.find_last_of('/');
        if (pos != std::string::npos)
            ret.erase(ret.begin(), ret.begin() + pos + 1);
        ret = new_path + "/" + ret;
    }
    else
    {
        std::size_t pos = ret.find_last_of('/');
        std::string begin_path = std::string(ret.begin(), ret.begin() + pos);
        ret = begin_path + "/" + new_path + "/" + std::string(ret.begin() + pos + 1, ret.end());
    }
    return ret;
}

std::string
Utils::remove_extension_in_filename(const std::string& filename)
{
    std::string ret;

    std::size_t pos = filename.find_last_of(".");
    if (pos == std::string::npos)
        return filename;
    ret = filename;
    ret.erase(ret.begin() + pos, ret.end());
    return ret;
}

bool
Utils::compare_source_timestamp(const std::string& sourceFilename, const std::string& objectFilename)
{
    struct stat ssource;
    struct stat sobject;

    if (stat(std::string(sourceFilename.begin(), sourceFilename.end() - 1).c_str(), &ssource) == -1)
        return NOT_RECOMPILE;
    if (stat(objectFilename.c_str(), &sobject) == -1)
        return RECOMPILE;
    if (ssource.st_mtimespec.tv_sec > sobject.st_mtimespec.tv_sec)
        return RECOMPILE;
    return NOT_RECOMPILE;
}

bool
Utils::compare_dependencies_timestamps(const std::string& objectFilename, const std::string& dependenciesFilename)
{
    struct stat                 sobject;
    struct stat                 sdepend;
    std::vector<std::string>    dependenceFiles;

    if (parse_dependencies_file(dependenceFiles, dependenciesFilename) == RECOMPILE)
        return RECOMPILE;
    if (dependenceFiles.size() == 0)
        return NOT_RECOMPILE;
    else if (stat(objectFilename.c_str(), &sobject) == -1)
        return NOT_RECOMPILE;
    for (unsigned long i = 0 ; i < dependenceFiles.size() ; ++i)
    {
        if (stat(dependenceFiles[i].c_str(), &sdepend) == -1)
            return RECOMPILE;
        else if (sobject.st_mtimespec.tv_sec < sdepend.st_mtimespec.tv_sec)
            return RECOMPILE;
    }
    return NOT_RECOMPILE;
}

unsigned long
Utils::get_file_size(FILE* file)
{
    struct stat s;
    if (fstat(fileno(file), &s) == -1)
        return 0;
    return s.st_size;
}

bool
Utils::parse_dependencies_file(std::vector<std::string>& dependenceFiles, const std::string& dependenciesFilename)
{
    FILE*           file = fopen(dependenciesFilename.c_str(), "r");
    if (!file)
        return RECOMPILE;
    unsigned long   file_size = Utils::get_file_size(file);

    if (file_size == 0)
        CLOSE_FILE_AND_RETURN(file)
    char*   data = (char*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fileno(file), 0);
    if (data <= (char*)NULL)
        CLOSE_FILE_AND_RETURN(file)
    else
    {
        if (data[0] != '\n')
        {
            int i = 0;
            for ( ; data[i] ; ++i)
            {
                if (data[i] == '\n' && data[i - 1] != '\\')
                {
                    ++i;
                    break;
                }
            }
            std::string dependenciesFile;
            for ( ; data[i] ; ++i)
            {
                if (data[i + 1] == '\n' && data[i] == ':')
                {
                    dependenceFiles.emplace_back(dependenciesFile);
                    dependenciesFile = "";
                }
                else
                    dependenciesFile.push_back(data[i]);
            }
        }
    }
    munmap(data, file_size);
    fclose(file);
    return NOT_RECOMPILE;
}

char*
Utils::dirname(char* path)
{
	char*	dirname = strrchr(path, '/');

	if (!dirname)
		return (".");
	*(dirname + 1) = '\0';
	return (path);
}
