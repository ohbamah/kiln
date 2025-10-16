#ifndef KILN_HPP
#define KILN_HPP

# define KR_CLEAN       KR_CLEAN_RUN, KR_CLEAN_DESC
# define KR_CLEAN_RUN   "rm -rf $OBJECTS_PATH"
# define KR_CLEAN_DESC  "Delete directory of all object files (can be dangerous if object files is relative to source files)"

# define KR_OCLEAN       KR_OCLEAN_RUN, KR_OCLEAN_DESC
# define KR_OCLEAN_RUN   "rm -f $OUTPUT_PATH/$OUTPUT_FILENAME"
# define KR_OCLEAN_DESC  "Delete output file (binary/library)"

# define KR_FCLEAN       KR_FCLEAN_RUN, KR_FCLEAN_DESC
# define KR_FCLEAN_RUN   "$(oclean)\n$(clean)"
# define KR_FCLEAN_DESC  "Delete all object files and generated output (binary/library)"

# define KR_RE       KR_RE_RUN, KR_RE_DESC
# define KR_RE_RUN   "$(oclean)\n$(clean)\n$(base)"
# define KR_RE_DESC  "Delete all object files and generated output (binary/library) and then compile"

# include "nodes.hpp"
# include "structs.hpp"
# include "utils.hpp"
# include "print.hpp"

# include <sys/wait.h>
# include <cstdlib>
# include <unistd.h>
# include <map>
# include <vector>
# include <string>

extern "C" {
    # include "hopt.h"
}

class Kiln
{
public:
    typedef std::pair<const char*, const char*>   TRulesAndDescription;

    static
    void
    AddToEnv(const Node& env);

    static
    void*
    find_or_create_shell_subcommand(void* arg);

    static
    void
    construct_output_command(KilnData& kilnData);

    static
    void
    print_final_commands(const KilnData& kilnData);

    static
    int
    execute_final_commands(Hopt& opt, KilnData& kilnData);

    static
    void
    create_mkdir_p_command(KilnData& kilnData);

    static
    void
    create_final_command(KilnData& kilnData);

    inline
    bool
    static
    at_least_one_subcommand_has_been_executed = false;

    static
    std::map<std::string, Kiln::TRulesAndDescription>
    rules_map;

private:

    static
    void
    ParseRuleCallsInRules(Hopt& opt, std::string&);
};

#endif
