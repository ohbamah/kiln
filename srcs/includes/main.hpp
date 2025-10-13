#ifndef KILN_MAIN_HPP
#define KILN_MAIN_HPP

# define DEFAULT_CONFIG_FILENAME    (char*)"./kiln.yml"

# include "nodes.hpp"
# include "kiln.hpp"
# include "fetch.hpp"
# include "generate.hpp"
# include "structs.hpp"
# include "utils.hpp"
# include "watch.hpp"
# include "DateTime.hpp"

extern "C" {
    # include "hopt.h"
    # include <stdlib.h>
    # include <unistd.h>
    # include <string.h>
    # include <stdio.h>
}

# include <map>
# include <string>

#endif
