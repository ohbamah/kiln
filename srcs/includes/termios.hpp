#ifndef KILN_TERMIOS_HPP
#define KILN_TERMIOS_HPP

# include <termios.h>
# include <unistd.h>
# include <fcntl.h>
# include <string>

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

    static
    const std::string&
    GetBuffer(void);

    static
    void
    EraseBuffer(void);

    static
    void
    EraseChars(int n);

    static
    void
    EraseLine(void);

    static
    void
    EraseToEndOfLine(void);

private:
    static
    void
    AddKeyToBuffer(char c);

    static inline
    std::string     buffer;

    static inline
    struct termios  old_termios;

    static inline
    bool            raw_mode_enabled = false;
};

#endif
