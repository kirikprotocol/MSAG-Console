#ifndef SCAG_TRANSPORT_HTTP_COMMAND
#define SCAG_TRANSPORT_HTTP_COMMAND

#include <string>

#include "scag/transport/SCAGCommand.h"

namespace scag { namespace transport { namespace http 
{
    using namespace scag::transport;
    
    /**
     * Service class for iterating field's names.
     */
    struct FieldIterator 
    {
    	bool next();
    	const std::string& value();
    };
    
    /**
     * Abstract class represents generic HttpCommand (Request or Response)
     * Defines basic command methods
     */
    class HttpCommand : public SCAGCommand
    {
    protected:

        HttpCommand() : SCAGCommand() {}
        
    public:

        virtual ~HttpCommand() {}
	
        virtual TransportType getType() const 
        { 
            return HTTP; 
        }
        
        // All values accessible by command context methods
        // should be passed to HttpResponce from HttpRequest 

        // ---------- Command context methods (start) --------
        virtual int getRuleId(); // initialy -1
        virtual void setRuleId(int ruleId);    
	
        virtual const std::string& getRouteId();
        virtual void setRouteId(const std::string& routeId);
	
    	virtual int64_t getOperationId(); // initialy -1
    	virtual void setOperationId(int64_t op);
        
        uint16_t getUSR();

    	const std::string& getAbonent();
        // ---------- Command context methods (end) ----------
        
    	// HTTP message header fields accessors
        FieldIterator& getHeaderFieldNames();
    	const std::string& getHeaderField(const std::string& fieldName);
    	void setHeaderField(const std::string& fieldName, 
                            const std::string& fieldValue);
    	
    	//TODO: add more Request/Responce packet fields accessors (getXXX/setXXX)
    };
    
    /**
     * HTTP Request methods
     */ 
    enum HttpMethod {
    	GET = 1, HEAD = 2, POST = 3, PUT = 4,
    	DELETE = 5, TRACE = 6, CONNECT = 7
    };
    
    /**
     * Class provides access to HTTP request message
     */
    class HttpRequest : public HttpCommand
    {
    public:

        HttpRequest() : HttpCommand() {}
        virtual ~HttpRequest() {}

    	HttpMethod getMethod();
    
    	const std::string& getSite();
    	const std::string& getSitePath();
    	unsigned int getSitePort();
    	const std::string& getSiteQuery();
    	const std::string& getSiteFull();
    	
    	//TODO: add more Request packet fields accessors (getXXX/setXXX)
    };
    
    /**
     * Class provides access to HTTP response message
     */
    class HttpResponse : public HttpCommand
    {
    public:

        HttpResponse() : HttpCommand() {}
        virtual ~HttpResponse() {}

        int getStatus();
        const std::string& getStatusLine();

    	int getContentLength();
    	std::string * getMessageText(); // applied to text/* content types only
    	bool getMessageBinary(void* &body, int& length); // applied to other contentent types
	
        //TODO: add more Response packet fields accessors (getXXX/setXXX)
    };
    
}}}

#endif // SCAG_TRANSPORT_HTTP_COMMAND