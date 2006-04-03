#include "HttpAdapter.h"

namespace scag { namespace re { namespace http {

using namespace scag::util::properties;

/*request_method r/w
request_site r/w
request_path r/w
reuqest_port r/w
request_query r/w
reuqest_site_full r/w
request_param_%s r
request_header_%s r/w
request_body_text r
request_body_binary r
request_body_

response_status r/w
response_headers_%s r/w*/


AccessType HttpCommandAdapter::CheckAccess(int handlerType,const std::string& name)
{
    return atNoAccess;
}

HttpCommandAdapter::~HttpCommandAdapter()
{
}

Property* HttpCommandAdapter::getProperty(const std::string& name)
{
    return 0;
}

void HttpCommandAdapter::changed(AdapterProperty& property)
{
}

}}}
