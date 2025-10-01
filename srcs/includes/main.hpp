#ifndef KILN_MAIN_HPP
#define KILN_MAIN_HPP

# define DEFAULT_CONFIG_FILENAME    (char*)"./clay.yml"

# include "nodes.hpp"

extern "C" {
    # include "hopt.h"
    # include <stdlib.h>
    # include <unistd.h>
    # include <string.h>
    # include <stdio.h>
}

# include <map>
# include <string>

# include "kiln.hpp"

#endif
