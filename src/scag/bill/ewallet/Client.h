#ifndef SCAG_BILL_EWALLET_CLIENT_H
#define SCAG_BILL_EWALLET_CLIENT_H

#include <memory>

namespace scag2 {
namespace bill {
namespace ewallet {

class Request;
class Response;
class Exception;

/// an interface to a ewallet client
class Client
{
public:
    class ResponseHandler
    {
    public:
        virtual ~ResponseHandler() {}
        virtual void handleResponse( std::auto_ptr<Request> request, std::auto_ptr<Response> resp ) = 0;
        virtual void handleError( std::auto_ptr<Request> request, const Exception& exc ) = 0;
    };

public:
    virtual ~Client() {}

    virtual void startup() = 0;
    virtual void shutdown() = 0;
    virtual bool canProcessRequest( Exception* exc = 0 ) = 0;

    /// process request asynchronously.
    /// handler must live until the request is processed (or failed).
    virtual void processRequest( std::auto_ptr<Request> request, ResponseHandler& handler ) = 0;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_CLIENT_H */
