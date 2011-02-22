#ifndef NULLREFERENCEEXCEPTION_H
#define NULLREFERENCEEXCEPTION_H

#include <stdexcept>

class NullReferenceException : public std::logic_error
{
public:
    /** Default constructor */
    explicit NullReferenceException(const std::string& What):
        std::logic_error(What)
    {
    }
    /** Default destructor */
    virtual ~NullReferenceException() throw()
    {
    }
protected:
private:
};

#endif // NULLREFERENCEEXCEPTION_H
