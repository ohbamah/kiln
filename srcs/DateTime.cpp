#include "DateTime.hpp"

// Fonction helper pour obtenir le nombre de jours dans un mois
static
int
getDaysInMonth(unsigned int month, unsigned int year) {
    if (month < 1 || month > 12) return 30; // Fallback
    
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Vérifier année bissextile pour février
    if (month == 2) {
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return isLeap ? 29 : 28;
    }
    
    return daysInMonth[month - 1]; // month est 1-12, array est 0-11
}

static
std::string
replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}

DateTime::DateTime(unsigned int    years,
                   unsigned int    months,
                   unsigned int    days,
                   unsigned int    hours,
                   unsigned int    minutes,
                   unsigned int    seconds,
                   unsigned int    useconds)
    : years(years), months(months), days(days), hours(hours), minutes(minutes), seconds(seconds), useconds(useconds)
{}

DateTime::DateTime(DateKeyArgument&& keyArguments)
{
    this->Change(static_cast<DateKeyArgument&&>(keyArguments));
}

DateTime::DateTime(DateTime&& dateTime)
{
    this->years = dateTime.years;
    this->months = dateTime.months;
    this->days = dateTime.days;
    this->hours = dateTime.hours;
    this->minutes = dateTime.minutes;
    this->seconds = dateTime.seconds;
    this->useconds = dateTime.useconds;
}

DateTime::DateTime(const DateTime& dateTime)
{
    this->years = dateTime.years;
    this->months = dateTime.months;
    this->days = dateTime.days;
    this->hours = dateTime.hours;
    this->minutes = dateTime.minutes;
    this->seconds = dateTime.seconds;
    this->useconds = dateTime.useconds;
}

DateTime::DateTime(const char* dateStr, const char* format)
{
    // A faire
    (void)dateStr;
    (void)format;
}

DateTime::~DateTime()
{}

DateTime
DateTime::Current(void)
{
    DateTime    to_return;

    time_t      raw_time;
    struct tm*  time_info;

    if (!DateTime::freezed || !DateTime::recorded || !DateTime::freezed_time_info)
    {
        time(&raw_time);
        time_info = localtime(&raw_time);
        DateTime::freezed_time_info = time_info;
        DateTime::recorded = true;
    }
    else
        time_info = DateTime::freezed_time_info;

    to_return.years = time_info->tm_year + 1900;
    to_return.months = time_info->tm_mon + 1;
    to_return.days = time_info->tm_mday;
    to_return.hours = time_info->tm_hour;
    to_return.minutes = time_info->tm_min;
    to_return.seconds = time_info->tm_sec;
    to_return.zone = time_info->tm_zone;

    if (!DateTime::freezed || !DateTime::recorded || DateTime::freezed_useconds == -1)
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        to_return.useconds = ts.tv_nsec / 1000000;
        DateTime::freezed_useconds = to_return.useconds;
    }
    else
        to_return.useconds = DateTime::freezed_useconds;

    return to_return;
}

DateTime*
DateTime::CurrentPtr(void)
{
    DateTime*    to_return = new DateTime;

    time_t      raw_time;
    struct tm*  time_info;

    time(&raw_time);
    time_info = localtime(&raw_time);

    to_return->years = time_info->tm_year + 1900;
    to_return->months = time_info->tm_mon + 1;
    to_return->days = time_info->tm_mday;
    to_return->hours = time_info->tm_hour;
    to_return->minutes = time_info->tm_min;
    to_return->seconds = time_info->tm_sec;
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    to_return->useconds = ts.tv_nsec / 1000000;

    return to_return;
}

const char*
DateTime::ToStrf(const char* format)
{
    std::string formatStr = format;

    formatStr = replaceAll(formatStr, "%Y", std::to_string(this->years));
    formatStr = replaceAll(formatStr, "%M", std::to_string(this->months));
    formatStr = replaceAll(formatStr, "%D", std::to_string(this->days));
    formatStr = replaceAll(formatStr, "%h", std::to_string(this->hours));
    formatStr = replaceAll(formatStr, "%m", std::to_string(this->minutes));
    formatStr = replaceAll(formatStr, "%s", std::to_string(this->seconds));
    formatStr = replaceAll(formatStr, "%u", std::to_string(this->useconds));

    return strdup(formatStr.c_str());
}

DateTime&
DateTime::Change(DateKeyArgument&& keyArguments)
{
    this->years = keyArguments.years;
    this->months = keyArguments.months;
    this->days = keyArguments.days;
    this->hours = keyArguments.hours;
    this->minutes = keyArguments.minutes;
    this->seconds = keyArguments.seconds;
    this->useconds = keyArguments.useconds;

    return *this;
}

DateTime&
DateTime::Years(unsigned int years)
{
    this->years = years;
    return *this;
}

DateTime&
DateTime::Months(unsigned int months)
{
    this->months = months;
    return *this;
}

DateTime&
DateTime::Days(unsigned int days)
{
    this->days = days;
    return *this;
}

DateTime&
DateTime::Hours(unsigned int hours)
{
    this->hours = hours;
    return *this;
}

DateTime&
DateTime::Minutes(unsigned int minutes)
{
    this->minutes = minutes;
    return *this;
}

DateTime&
DateTime::Seconds(unsigned int seconds)
{
    this->seconds = seconds;
    return *this;
}

DateTime&
DateTime::Milliseconds(unsigned int milliseconds)
{
    this->useconds = milliseconds;
    return *this;
}

DateTime
DateTime::FromNow(unsigned int years,
                  unsigned int months,
                  unsigned int days,
                  unsigned int hours,
                  unsigned int minutes,
                  unsigned int seconds,
                  unsigned int useconds) const
{
    DateTime to_return = *this;
    
    // Ajouter les millisecondes avec gestion des dépassements
    to_return.useconds += useconds;
    if (to_return.useconds >= 1000) {
        seconds += to_return.useconds / 1000;
        to_return.useconds %= 1000;
    }
    
    // Ajouter les secondes
    to_return.seconds += seconds;
    if (to_return.seconds >= 60) {
        minutes += to_return.seconds / 60;
        to_return.seconds %= 60;
    }
    
    // Ajouter les minutes
    to_return.minutes += minutes;
    if (to_return.minutes >= 60) {
        hours += to_return.minutes / 60;
        to_return.minutes %= 60;
    }
    
    // Ajouter les heures
    to_return.hours += hours;
    if (to_return.hours >= 24) {
        days += to_return.hours / 24;
        to_return.hours %= 24;
    }
    
    // Ajouter les jours (plus complexe à cause des mois variables)
    to_return.days += days;
    while (to_return.days > (unsigned int)getDaysInMonth(to_return.months, to_return.years)) {
        to_return.days -= getDaysInMonth(to_return.months, to_return.years);
        to_return.months++;
        if (to_return.months > 12) {
            to_return.months = 1;
            to_return.years++;
        }
    }
    
    // Ajouter les mois
    to_return.months += months;
    while (to_return.months > 12) {
        to_return.months -= 12;
        to_return.years++;
    }
    
    // Ajouter les années
    to_return.years += years;
    
    return to_return;
}

DateTime
DateTime::Ago(unsigned int years,
              unsigned int months,
              unsigned int days,
              unsigned int hours,
              unsigned int minutes,
              unsigned int seconds,
              unsigned int useconds) const
{
    DateTime to_return = *this;
    
    // Soustraire les millisecondes
    if (to_return.useconds < useconds) {
        to_return.useconds += 1000;
        seconds++;
    }
    to_return.useconds -= useconds;
    
    // Soustraire les secondes
    if (to_return.seconds < seconds) {
        to_return.seconds += 60;
        minutes++;
    }
    to_return.seconds -= seconds;
    
    // Soustraire les minutes
    if (to_return.minutes < minutes) {
        to_return.minutes += 60;
        hours++;
    }
    to_return.minutes -= minutes;
    
    // Soustraire les heures
    if (to_return.hours < hours) {
        to_return.hours += 24;
        days++;
    }
    to_return.hours -= hours;
    
    // Soustraire les jours
    while (to_return.days <= days) {
        to_return.months--;
        if (to_return.months < 1) {
            to_return.months = 12;
            to_return.years--;
        }
        to_return.days += getDaysInMonth(to_return.months, to_return.years);
    }
    to_return.days -= days;
    
    // Soustraire les mois
    if (to_return.months <= months) {
        unsigned int monthsToSubtract = months;
        while (to_return.months <= monthsToSubtract) {
            monthsToSubtract -= to_return.months;
            to_return.years--;
            to_return.months = 12;
        }
        to_return.months -= monthsToSubtract;
    } else {
        to_return.months -= months;
    }
    
    // Soustraire les années
    to_return.years -= years;
    
    return to_return;
}

DateTime
DateTime::FromNow(const DateKeyArgument& keyArguments) const
{
    return this->FromNow(keyArguments.years,
                         keyArguments.months,
                         keyArguments.days,
                         keyArguments.hours,
                         keyArguments.minutes,
                         keyArguments.seconds,
                         keyArguments.useconds);
}

DateTime
DateTime::Ago(const DateKeyArgument& keyArguments) const
{
    return this->Ago(keyArguments.years,
                     keyArguments.months,
                     keyArguments.days,
                     keyArguments.hours,
                     keyArguments.minutes,
                     keyArguments.seconds,
                     keyArguments.useconds);
}

void
DateTime::Freeze(void)
{
    DateTime::freezed = true;
}

void
DateTime::Defreeze(void)
{
    DateTime::freezed = false;
    DateTime::recorded = false;
}

bool
DateTime::operator <(const DateTime& dateTime) const
{
    return (this->years < dateTime.years ||
            this->months < dateTime.months ||
            this->days < dateTime.days ||
            this->hours < dateTime.hours ||
            this->minutes < dateTime.minutes ||
            this->seconds < dateTime.seconds ||
            this->useconds < dateTime.useconds);
}

bool
DateTime::operator <=(const DateTime& dateTime) const
{
    return (this->years <= dateTime.years ||
        this->months <= dateTime.months ||
        this->days <= dateTime.days ||
        this->hours <= dateTime.hours ||
        this->minutes <= dateTime.minutes ||
        this->seconds <= dateTime.seconds ||
        this->useconds <= dateTime.useconds);
}

bool
DateTime::operator ==(const DateTime& dateTime) const
{
    return (this->years == dateTime.years ||
        this->months == dateTime.months ||
        this->days == dateTime.days ||
        this->hours == dateTime.hours ||
        this->minutes == dateTime.minutes ||
        this->seconds == dateTime.seconds ||
        this->useconds == dateTime.useconds);
}

bool
DateTime::operator !=(const DateTime& dateTime) const
{
    return (this->years != dateTime.years ||
        this->months != dateTime.months ||
        this->days != dateTime.days ||
        this->hours != dateTime.hours ||
        this->minutes != dateTime.minutes ||
        this->seconds != dateTime.seconds ||
        this->useconds != dateTime.useconds);
}

bool
DateTime::operator >=(const DateTime& dateTime) const
{
    return (this->years >= dateTime.years ||
        this->months >= dateTime.months ||
        this->days >= dateTime.days ||
        this->hours >= dateTime.hours ||
        this->minutes >= dateTime.minutes ||
        this->seconds >= dateTime.seconds ||
        this->useconds >= dateTime.useconds);
}

bool
DateTime::operator >(const DateTime& dateTime) const
{
    return (this->years > dateTime.years ||
        this->months > dateTime.months ||
        this->days > dateTime.days ||
        this->hours > dateTime.hours ||
        this->minutes > dateTime.minutes ||
        this->seconds > dateTime.seconds ||
        this->useconds > dateTime.useconds);
}

DateTime&
DateTime::operator =(const DateTime& dateTime)
{
    this->years = dateTime.years;
    this->months = dateTime.months;
    this->days = dateTime.days;
    this->hours = dateTime.hours;
    this->minutes = dateTime.minutes;
    this->seconds = dateTime.seconds;
    this->useconds = dateTime.useconds;

    return *this;
}

DateTime&
DateTime::operator =(DateTime&& dateTime)
{
    this->years = dateTime.years;
    this->months = dateTime.months;
    this->days = dateTime.days;
    this->hours = dateTime.hours;
    this->minutes = dateTime.minutes;
    this->seconds = dateTime.seconds;
    this->useconds = dateTime.useconds;

    return *this;
}
