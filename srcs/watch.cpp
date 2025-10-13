#include "watch.hpp"

// Ouvrir tous les .d, prendre les fichiers .h et les ajouter à la liste des fichiers à surveiller (liste differente des sources)
// À chaque re-compilation d'un fichier source, ré-ouvrir le .d et re-récupérer les dépendances, et ainsi de suite ...

// Attendre un certain nombre de milliseconde à chaque recompilation
// Si un fichier est modifié alors que Kiln est entrain de recompiler, annuler la recompilation et re-recompiler

// Faire pagination si trop de logs
void
Watch::Call(const Node& config, Hopt& opt, KilnData& kilnData)
{
    Watch::config = config;

    Watch::Init(opt);
    Watch::Invoke(opt, kilnData);
    Watch::Destroy();
}

void
Watch::SignalHandler(int signum)
{
    (void)signum;
    Watch::Exit(130);
}

void
Watch::Signal(void)
{
    signal(SIGINT, Watch::SignalHandler);
}

void
Watch::Invoke(Hopt& opt, KilnData& kilnData)
{
    char            key;
    struct kevent   changeList[MAX_FILES];

    // Créer la kqueue
    Watch::kq = kqueue();
    if (Watch::kq == -1) {
        perror("kqueue");
        exit(1);
    }

    Watch::FetchFilesToSpy(changeList, kilnData);
    Watch::RegisterEvents(changeList);
    // Watch::AddLog(std::to_string(Watch::filepaths.size()) + " files to spy");
    
    while (1)
    {
        struct kevent   events[MAX_FILES];

        if ((key = Termios::GetKey()))
            Watch::InterpretKey(opt, kilnData, key);
        
        int nev = kevent(Watch::kq, NULL, 0, events, MAX_FILES, NULL);
        if (nev == -1) {
            perror("kevent");
            exit(14);
        }

        if (nev > 0) {  
            memset(&kilnData, 0, sizeof(KilnData));
            handle_final_command(kilnData, Watch::config);
            Watch::ExecuteBaseRule(opt, kilnData);
        }
    }
}
 
void
Watch::FetchFilesToSpy(struct kevent changeList[MAX_FILES], KilnData& kilnData)
{
    std::vector<std::string>    dependenciesFiles;

    Watch::fds.clear();
    Watch::filepaths = kilnData.sourceFiles;
    for (unsigned int i = 0 ; i < kilnData.headerDependenciesFiles.size() ; ++i)
    {
        Utils::parse_dependencies_file(dependenciesFiles, kilnData.headerDependenciesFiles[i]);
        Watch::filepaths.insert(Watch::filepaths.end(), 
                                dependenciesFiles.begin(), 
                                dependenciesFiles.end());
        dependenciesFiles.clear();
    }
    
    for (unsigned int i = 0 ; i < Watch::filepaths.size() ; ++i)
    {

        int fd = open(Watch::filepaths[i].data(), O_EVTONLY);
        if (fd == -1) {
            perror(Watch::filepaths[i].c_str());
            continue;
        }

        // Do special thing on RENAMING and REVOKING of a file
        Watch::fds.emplace_back(fd);
        EV_SET(&changeList[i], fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR,
               NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND,
               0, NULL);
    }
}

void
Watch::RegisterEvents(struct kevent changeList[])
{
    kevent(Watch::kq, changeList, Watch::fds.size(), NULL, 0, NULL);
}

void
Watch::UnregisterEvents()
{
    for (unsigned int i = 0 ; i < Watch::fds.size() ; ++i)
        close(Watch::fds[i]);
    close(Watch::kq);
}

void
Watch::ExecuteBaseRule(Hopt& opt, KilnData& kilnData)
{
    std::string format;
    DateTime    startTime;
    DateTime    endTime;

    // Watch::AddLog("Files changed...");

    startTime = DateTime::Current();
    base_rule(opt, kilnData);
    endTime = DateTime::Current();

    unsigned int    useconds = ((endTime.hours - startTime.hours) * 60 * 60 * 1000) +
                               ((endTime.minutes - startTime.minutes) * 60 * 1000) +
                               ((endTime.seconds - startTime.seconds) * 1000) +
                               ((endTime.useconds - startTime.useconds));
    unsigned int    seconds = ((endTime.hours - startTime.hours) * 60 * 60) +
                              ((endTime.minutes - startTime.minutes) * 60) +
                              ((endTime.seconds - startTime.seconds)) +
                              ((endTime.useconds - startTime.useconds) / 1000);
    if (useconds > 1000)
        useconds -=  1000 * (useconds / 1000);
    if (useconds < 10)
        format = "000" + std::to_string(useconds);
    else if (useconds < 100)
        format = "00" + std::to_string(useconds);
    else if (useconds < 1000)
        format = "0" + std::to_string(useconds);
    // Watch::AddLog("Compilation done in " + std::to_string(seconds) + "," + format + "s");
}

void
Watch::CloseFiles(void)
{
    for (unsigned int i = 0 ; i < Watch::fds.size() ; ++i)
        close(Watch::fds[i]);
}

void
Watch::Init(Hopt& opt)
{
    // opt.no_format = true;
    // opt.no_messages = true;
    // opt.no_scripts = true;
    setvbuf(stdout, NULL, _IOLBF, 0);
    Watch::Signal();
    Watch::startingAt = DateTime::Current().ToStrf("%Y-%M-%D %h:%m:%s");
    // Watch::DisplayInterface();
    Termios::Capture();
}

void
Watch::Destroy(void)
{
    // Watch::AddLog("Watcher terminated at " + std::string(DateTime::Current().ToStrf("%Y-%M-%D %h:%m:%s")));
    fflush(stdout);
    Watch::UnregisterEvents();
    Termios::Reset();
}

void
Watch::InterpretKey(Hopt& opt, KilnData& kilnData, char key)
{
    switch (key)
    {
        case 'Q':
        case 'q':
            Watch::Exit();
            break;
        case 'R':
        case 'r':
            Watch::ExecuteBaseRule(opt, kilnData);
            break;
        case 'C':
        case 'c':
            Watch::logs.clear();
            Watch::ForceRefresh();
            break;
        case 'T':
        case 't':
            Watch::AddLog("Test log");
            break;
        case 'V':
        case 'v':
            break;
        case 'f':  // Force refresh
        case 'F':
            Watch::ForceRefresh();
            break;
        case 'P':
        case 'p':
            break;
    }
}

void
Watch::AddLog(const std::string& log)
{
    Watch::logs.emplace_back(log);
    Watch::ForceRefresh();
}

void
Watch::ForceRefresh(void)
{
    Watch::ClearScreen();
    Watch::DisplayInterface();
    // fflush(stdout);
}

void
Watch::ClearScreen(void)
{
    printf("\033[3J\033[2J\033[H");
    // fflush(stdout);
}

void
Watch::DisplayInterface(void)
{
    Watch::ClearScreen();
    
    // Header
    printf("\033[1;36m╔═══════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║\033[0m                    \033[1;34mKILN FILE WATCHER\033[0m                          \033[1;36m║\033[0m\n");
    printf("\033[1;36m╚═══════════════════════════════════════════════════════════════╝\033[0m\n\n");
    
    // Status section;
    printf("\033[1;37m🕐 Started:\033[0m %s\n", Watch::startingAt);
    
    printf("\n\033[1;36m" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "\033[0m\n");
    
    // Activity log
    printf("\033[1;37m📋 ACTIVITY LOG:\033[0m\n\n");
    for (unsigned int i = 0 ; i < Watch::logs.size() ; ++i)
        printf("\033[0;37m%s\033[0m\n", Watch::logs[i].c_str());

    printf("\n\033[1;36m" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "─" "\033[0m\n");

    printf("\033[1;37m🎮 CONTROLS:\033[0m\n");
    printf("  \033[1;33mCtrl+C\033[0m  Stop watching\n");
    printf("  \033[1;33mq\033[0m       Quit\n");
    printf("  \033[1;33mt\033[0m       Test log\n");
    printf("  \033[1;33mr\033[0m       Force rebuild\n");
    printf("  \033[1;33mc\033[0m       Clear log\n");
    printf("  \033[1;33mf\033[0m       Force refresh\n");
    printf("  \033[1;33mv\033[0m       Verbosity\n");
    printf("  \033[1;33mp\033[0m       Parse config file\n");

    printf("\n\033[1;32m⚡ Watching for changes... Press Ctrl+C to stop\033[0m\n");
    // fflush(stdout);
}

void
Watch::Exit(int code)
{
    Watch::Destroy();
    exit(code);
}
