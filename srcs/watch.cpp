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
            // std::cout << "Files changed, checking what needs recompilation...\n";
            kilnData.clear();
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

    Watch::result_status = COMPILATION;
    Watch::AddLog("Compilation in progress...");

    startTime = DateTime::Current();
    Watch::result_status = base_rule(opt, kilnData); 
    endTime = DateTime::Current();

    long long start_ms = (long long)startTime.hours * 3600000 + 
                         (long long)startTime.minutes * 60000 + 
                         (long long)startTime.seconds * 1000 + 
                         (long long)startTime.useconds;
    
    long long end_ms = (long long)endTime.hours * 3600000 + 
                       (long long)endTime.minutes * 60000 + 
                       (long long)endTime.seconds * 1000 + 
                       (long long)endTime.useconds;

    long long diff_ms = end_ms - start_ms;
    if (diff_ms < 0) diff_ms = 0;
    
    unsigned int seconds = diff_ms / 1000;
    unsigned int useconds = diff_ms % 1000;
    if (useconds < 10)
        format = "000" + std::to_string(useconds);
    else if (useconds < 100)
        format = "00" + std::to_string(useconds);
    else if (useconds < 1000)
        format = "0" + std::to_string(useconds);

    Watch::AddLog("Compilation done in " + std::to_string(seconds) + "," + format + "s");
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
    opt.no_format = true;
    Watch::Signal();
    Watch::startingAt = DateTime::Current().ToStrf("%Y-%M-%D %h:%m:%s");
    Watch::DisplayInterface();
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
    if (key == '\n' || key == '\r')
    {
        Rule         rule;
        const
        std::string& buffer = Termios::GetBuffer();

        rule.opt = &opt;
        rule.rulename = buffer;
        rule.run = "";
        Kiln::find_or_create_shell_subcommand((void*)&rule);

        Termios::EraseBuffer();
    }
}

void
Watch::AddLog(const std::string& log)
{
    Watch::result_log = log;
    Watch::ForceRefresh();
}

void
Watch::ForceRefresh(void)
{
    Watch::ClearScreen();
    Watch::DisplayInterface();
}

void
Watch::ClearScreen(void)
{
}

void
Watch::DisplayInterface(void)
{
    std::string buffer;

    buffer.reserve(5000);
    buffer = "";
    if (Watch::result_status != ERROR)
    {
        buffer += "\033[3J\033[2J\033[H"; 
        buffer += "\033[3J\033[2J\033[H"; 

        buffer += "\033[1;37m🕐 Started:\033[0m " + std::string(Watch::startingAt) + "\n\n";
    }

    if (!Watch::result_log.empty())
    {
        if (Watch::result_status == SUCCESS)
            buffer += "\033[1;32m";
        if (Watch::result_status == ERROR)
            buffer += "\033[1;31m";
        if (Watch::result_status == COMPILATION)
            buffer += "\033[1;33m";
        buffer += Watch::result_log + "\033[0m\n";
    }

    write(STDOUT_FILENO, buffer.data(), buffer.size());     
    fflush(stdout);
}

void
Watch::Exit(int code)
{
    Watch::Destroy();
    exit(code);
}
