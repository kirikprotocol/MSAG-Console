#ifndef STRORE_EXCEPTIONS_DECLARATIONS#define STRORE_EXCEPTIONS_DECLARATIONS

#include <exception>
using std::exception;
namespace smsc { namespace store {
    class AuthenticationException : public exception
    {
    public:
        AuthenticationException() {};
        virtual ~AuthenticationException() throw() {};
       
        virtual const char* what() const throw() {
            return "Unable to authenticate user to DB !";
        };    
    };
   
    class ResourceAllocationException : public exception
    {
    public:
        ResourceAllocationException() {};
        virtual ~ResourceAllocationException() throw() {};
        
        virtual const char* what() const throw() {
            return "Unable to allocate resources for DB !";
        };    
    };
    
    class NoSuchMessageException : public exception
    {
    public:
        NoSuchMessageException() {};
        virtual ~NoSuchMessageException() throw() {};
        
        virtual const char* what() const throw() {
            return "Unable to find message with such id in DB !";
        };    
    };

}}

#endif

