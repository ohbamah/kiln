#include "termios.hpp"

void
Termios::Capture(void)
{
    if (raw_mode_enabled)
        return ;
    
    tcgetattr(STDIN_FILENO, &Termios::old_termios);

    struct termios  new_termios = Termios::old_termios;
    new_termios.c_lflag &= ~(ICANON);
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
    {
        Termios::AddKeyToBuffer(c);
        return c;
    }
    return 0;
}

const std::string&
Termios::GetBuffer(void)
{
    return Termios::buffer;
}

void
Termios::EraseBuffer(void)
{
    Termios::EraseChars(Termios::buffer.size());
    Termios::buffer.clear();
}

void
Termios::AddKeyToBuffer(char c)
{
    Termios::buffer.push_back(c);
}

void
Termios::EraseChars(int n)
{
    printf("\033[%dP", n);
    fflush(stdout);
}

void
Termios::EraseLine(void)
{
    printf("\033[2K\r");
    fflush(stdout);
}

void
Termios::EraseToEndOfLine(void)
{
    printf("\033[K");
    fflush(stdout);
}
