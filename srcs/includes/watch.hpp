#ifndef KILN_WATCH_HPP
#define KILN_WATCH_HPP

# define MAX_FILES  1024

# define SUCCESS        0
# define ERROR          1
# define COMPILATION    2

# include "structs.hpp"
# include "DateTime.hpp"
# include "termios.hpp"
# include "utils.hpp"
# include "fetch.hpp"
# include "nodes.hpp"
# include "kiln.hpp"

# include <sys/types.h>
# include <sys/event.h>
# include <sys/time.h>
# include <fcntl.h>
# include <unistd.h>
# include <string>
# include <vector>

int
base_rule(Hopt& opt, KilnData& kilnData);

void
handle_final_command(KilnData& kilnData, Node& config);

class Watch
{
public:
    static
    void
    Call(const Node& config, Hopt& opt, KilnData& kilnData);

private:
    static
    void
    Signal(void);

    static
    void
    SignalHandler(int signum);

    static
    void
    Invoke(Hopt& opt, KilnData& kilnData);

    static
    void
    FetchFilesToSpy(struct kevent changeList[MAX_FILES], KilnData& kilnData);

    static
    void
    RegisterEvents(struct kevent changeList[]);

    static
    void
    UnregisterEvents();

    static
    void
    ExecuteBaseRule(Hopt& opt, KilnData& kilnData);

    static
    void
    Init(Hopt& opt);

    static
    void
    Destroy(void);

    static
    void
    InterpretKey(Hopt& opt, KilnData& kilnData, char key);

    static
    void
    CloseFiles(void);

    static
    void
    AddLog(const std::string& log);

    static
    void
    ForceRefresh(void);

    static
    void
    ClearScreen(void);

    static
    void
    AddControlsSection(void);

    static
    void
    DisplayInterface(void);

    static
    void
    Exit(int code = 0);

    static inline
    Node
    config;

    static inline
    int
    kq = kqueue();

    static inline
    std::vector<std::string>
    filepaths;

    static inline
    std::vector<int>
    fds;

    static inline
    const char*
    startingAt;

    static inline
    int
    fd = -1;

    static inline
    char
    paths[PATH_MAX] = { 0 };

    static inline
    int
    result_status = COMPILATION;

    static inline
    std::string
    result_log = "";
};

#endif
