#ifndef STRORE_EXCEPTIONS_DECLARATIONS
#define STRORE_EXCEPTIONS_DECLARATIONS

#include <stdio.h>
#include <string.h>
#include <exception>

using std::exception;

namespace smsc { namespace store
{
    const int MAX_MESSAGE_LENGTH = 1024;

	class StoreException : public exception
    {
	protected:
		
        char  cause[MAX_MESSAGE_LENGTH];
		
		StoreException() {};

	public:
		
		StoreException(const char* _cause) {
            sprintf(cause, "Failure cause - %s", _cause);
		};
        
		virtual ~StoreException() throw() {};
       
        virtual const char* what() const throw() {
            return cause;
        };
		
    };
	
	class StorageException : public StoreException
    {
    protected:
		
		int	  code;
		
		StorageException(int _code=-1) 
			: StoreException(), code(_code) {};

	public:
        
        StorageException(const char* _cause, int _code=-1) 
			: StoreException(), code(_code) 
		{
            sprintf(cause, "Code - %d, Failure cause - %s", _code, _cause);
		};
        virtual ~StorageException() throw() {};
		
		inline int getErrorCode() {
			return code;
		};
	};
	
	class ConnectFailureException : public StorageException
    {
	public:
		
		ConnectFailureException(StorageException& exc) 
			: StorageException(exc.getErrorCode())
		{
			strcpy(cause, exc.what());
		};
		virtual ~ConnectFailureException() throw() {};
	};
    
    class NoSuchMessageException : public StoreException
    {
    public:
        NoSuchMessageException() 
			: StoreException("Unable to find message with such id in DB !") {};
		virtual ~NoSuchMessageException() throw() {};
    };

}}

#endif

