#ifndef __INMANEMU_INMANCONF_H__
#define __INMANEMU_INMANCONF_H__

#include <string>

namespace inmanemu { namespace util { 

class InmanemuConfig
{
public:
    InmanemuConfig() : host("127.0.0.1") , port(8888) {};
    void Init();
    ~InmanemuConfig();

    std::string host;
    int port;
};

}}

#endif

