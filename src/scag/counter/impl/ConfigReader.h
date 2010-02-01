#ifndef _SCAG_COUNTER_IMPL_CONFIGREADER_H
#define _SCAG_COUNTER_IMPL_CONFIGREADER_H

#include <map>
#include <string>
#include "scag/counter/ActionTable.h"

namespace scag2 {
namespace counter {

class TemplateManager;

namespace impl {

class ConfigReader
{
    struct TemplateProto {
        CountType          countType;
        std::string        limitName;
        int                param0, param1;
    };

    typedef std::map<std::string,ActionList>    LimitMap;
    typedef std::map<std::string,TemplateProto> ProtoMap;

public:
    ConfigReader() : log_(smsc::logger::Logger::getInstance("cnt.cfg")) {}
    bool readConfig( const char* fname );
    void reload( TemplateManager& tmgr );

private:
    smsc::logger::Logger* log_;
    LimitMap              limitMap_;
    ProtoMap              protoMap_;
};

}
}
}

#endif /* !_SCAG_COUNTER_IMPL_CONFIGREADER_H */
