#ifndef STD_EXCEPTION_FORMAT_HPP
#define STD_EXCEPTION_FORMAT_HPP

# include <exception>
# include <stdarg.h>
# include <stdlib.h>
# include <stdio.h>

namespace std
{
    # define DECL_EXCEPTION_FORMAT(classN, ...)\
    class classN : public std::exception_format\
    {\
    public:\
        classN(void) : std::exception_format(__VA_ARGS__)\
        {}\
        classN(const char* pFormat, ...) : std::exception_format()\
        {\
            va_list list;\
            va_start(list, pFormat);\
            this->std::exception_format::create_format_message(pFormat, list);\
            va_end(list);\
        }\
        virtual ~classN() = default;\
    };

    class exception_format : public std::exception
    {
    public:
        exception_format(const char* format, ...)
        {
            va_list list;
            
            va_start(list, format);
            unsigned long size = vsnprintf(nullptr, 0, format, list);
            va_end(list);

            this->__what = (char*)malloc(size + 1);
            if (!this->__what)
                throw std::bad_alloc();

            va_start(list, format);
            vsnprintf(this->__what, size + 1, format, list);
            va_end(list);
        }

        ~exception_format()
        {
            free(this->__what);
        }

        virtual
        const char*
        what(void) const noexcept override
        {
            return this->__what;
        }

        exception_format& operator=(const exception_format& other)
        {
            if (this != &other)
            {
                free(this->__what);
                this->__what = strdup(other.__what);
            }
            return *this;
        }

    protected:
        void
        create_format_message(const char* format, va_list list)
        {
            va_list copy;
            va_copy(copy, list);
            unsigned long size = vsnprintf(nullptr, 0, format, copy);
            va_end(copy);

            this->__what = (char*)malloc(size + 1);
            if (!this->__what)
                throw std::bad_alloc();

            vsnprintf(this->__what, size + 1, format, list);
        }

        exception_format() = default;

    private:
        char* __what = nullptr;
    };

}

#endif
