#ifndef STRORE_EXCEPTIONS_DECLARATIONS
#define STRORE_EXCEPTIONS_DECLARATIONS

#include <stdio.h>
#include <string.h>
#include <exception>

using std::exception;

namespace smsc { namespace store
{
    class StoreException : public exception
    {
	private:
		
		int	  code;
		char  cause[512];

	public:
		
		StoreException(int _code, const char* _cause) : code(_code) {
			//cause = strdup(_cause);
			sprintf(cause, "Code - %d, Cause - %s", _code, _cause);
		};
        
		virtual ~StoreException() throw() {
			//if (cause) free(cause);
		};
       
        virtual const char* what() const throw() {
            return cause;
        };
		
		inline int getErrorCode() {
			return code;
		};
    };
    
	class AuthenticationException : public StoreException
    {
    public:
        AuthenticationException() : StoreException(-1, 
			"Unable to authenticate user to DB !") {};
        virtual ~AuthenticationException() throw() {};
    };
   
    class ResourceAllocationException : public StoreException
    {
    public:
        ResourceAllocationException() : StoreException(-1, 
			"Unable to allocate resources for DB !") {};
		virtual ~ResourceAllocationException() throw() {};
    };
    
    class NoSuchMessageException : public StoreException
    {
    public:
        NoSuchMessageException() : StoreException(-1, 
			"Unable to find message with such id in DB !") {};
		virtual ~NoSuchMessageException() throw() {};
    };

}}

#endif

