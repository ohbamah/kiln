#ifndef DATETIME_HPP
#define DATETIME_HPP

# include <ctime>
# include <string>

struct DateKeyArgument {
    unsigned int    years = 0;
    unsigned int    months = 0;
    unsigned int    days = 0;
    unsigned int    hours = 0;
    unsigned int    minutes = 0;
    unsigned int    seconds = 0;
    unsigned int    useconds = 0;
};

class DateTime
{
public:
    DateTime(unsigned int years = 1900,
             unsigned int months = 1,
             unsigned int days = 1,
             unsigned int hours = 1,
             unsigned int minutes = 0,
             unsigned int seconds = 0,
             unsigned int useconds = 0);
    DateTime(DateKeyArgument&& keyArguments);
    DateTime(DateTime&& dateTime);
    DateTime(const DateTime& dateTime);
    DateTime(const char* dateStr, const char* format);
    ~DateTime();

    static
    DateTime
    Current(void);

    // Can't be freezed
    static
    DateTime*
    CurrentPtr(void);

    // %Y --> year
    // %M --> month
    // %D --> day
    // %h --> hour
    // %m --> minute
    // %s --> second
    // %u --> millisecond
    const char*
    ToStrf(const char* format);

    DateTime&
    Change(DateKeyArgument&& keyArguments);

    DateTime&
    Years(unsigned int years);
    DateTime&
    Months(unsigned int months);
    DateTime&
    Days(unsigned int days);
    DateTime&
    Hours(unsigned int hours);
    DateTime&
    Minutes(unsigned int minutes);
    DateTime&
    Seconds(unsigned int seconds);
    DateTime&
    Milliseconds(unsigned int milliseconds);

        // To refactor for outbound range
    DateTime
    FromNow(unsigned int years = 0,
            unsigned int months = 0,
            unsigned int days = 0,
            unsigned int hours = 0,
            unsigned int minutes = 0,
            unsigned int seconds = 0,
            unsigned int useconds = 0) const;

        // To refactor for outbound range
    DateTime
    Ago(unsigned int years = 0,
        unsigned int months = 0,
        unsigned int days = 0,
        unsigned int hours = 0,
        unsigned int minutes = 0,
        unsigned int seconds = 0,
        unsigned int useconds = 0) const;

        // To refactor for outbound range
    DateTime
    FromNow(const DateKeyArgument& keyArguments) const;

        // To refactor for outbound range
    DateTime
    Ago(const DateKeyArgument& keyArguments) const;

    static
    void
    Freeze(void);

    static
    void
    Defreeze(void);

    bool
    operator <(const DateTime& dateTime) const;
    bool
    operator <=(const DateTime& dateTime) const;
    bool
    operator ==(const DateTime& dateTime) const;
    bool
    operator !=(const DateTime& dateTime) const;
    bool
    operator >=(const DateTime& dateTime) const;
    bool
    operator >(const DateTime& dateTime) const;

    DateTime&
    operator =(const DateTime& dateTime);

    DateTime&
    operator =(DateTime&& dateTime);

    char*           zone;
    unsigned int    years;
    unsigned int    months;
    unsigned int    days;
    unsigned int    hours;
    unsigned int    minutes;
    unsigned int    seconds;
    unsigned int    useconds;

private:
    
    inline static
    bool        freezed = false;
    
    inline static
    bool        recorded = false;
    
    inline static
    struct tm*  freezed_time_info = NULL;
    
    inline static
    int         freezed_useconds = -1;
};

#endif