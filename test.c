#include "includes/hopt.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int fun(int ac, char** av, void* ptr)
{
    char** test = (char**)ptr;
    *test = av[1];
    printf("fun_ac: %d\n", ac);
    for (int i = 0 ; i <= ac ; ++i)
        printf("%s ", av[i]);
    return (0);
}

int main(int ac, char** av)
{
    char    ahah = 0;
    char*   var_params[999] = { 0 };
    char*   ez[2] = { 0 };
    char*   miaou = 0;

    hopt_add_option("c", 2, HOPT_FLCB, fun, (void*)&miaou, "desc");
    hopt_add_option("s", 0, 0, &ahah, "desc");
    hopt_add_option("v", HOPT_VARIADIC_ARGUMENTS, 0, &var_params, "desc");
    int c = hopt(ac, av);
    if (c == -1)
    {
        printf("%s\n", hopt_strerror());
        return (1);
    }
    printf("\nword: %s\n", miaou);
    printf("options&args counted: %d\n", c);
    for (int i = 0 ; i < ac ; ++i)
        printf("%s ", av[i]);
    printf("\n");
    printf("ez: %s %s\n", ez[0], ez[1]);
    return (0);
}
