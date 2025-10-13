#ifndef KILN_TERMIOS_HPP
#define KILN_TERMIOS_HPP

# include <termios.h>
# include <unistd.h>
# include <fcntl.h>

class Termios
{
public:
    static
    void
    Capture(void);

    static
    void
    Reset(void);

    static
    char
    GetKey(void);

private:
    static inline
    struct termios  old_termios;

    static inline
    bool            raw_mode_enabled = false;
};

#endif
