#include "termios.hpp"

void
Termios::Capture(void)
{
    if (raw_mode_enabled)
        return ;
    
    tcgetattr(STDIN_FILENO, &Termios::old_termios);

    struct termios  new_termios = Termios::old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    int flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    raw_mode_enabled = true;
}

void
Termios::Reset(void)
{
    if (!raw_mode_enabled)
        return ;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &Termios::old_termios);

    int flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);

    raw_mode_enabled = false;
}

char
Termios::GetKey(void)
{
    char    c;
    if (read(STDIN_FILENO, &c, 1) == 1)
        return c;
    return 0;
}
