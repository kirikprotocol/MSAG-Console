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

    /// @param useExc if true then throw exception instead of returning false
    bool readConfig( const char* fname, bool useExc = false );
    void reload( TemplateManager& tmgr );

    void replaceObserver( TemplateManager& tmgr, const std::string& id ); // throw
    void replaceTemplate( TemplateManager& tmgr, const std::string& id ); // throw

private:
    smsc::logger::Logger* log_;
    LimitMap              limitMap_;
    ProtoMap              protoMap_;
};

}
}
}

#endif /* !_SCAG_COUNTER_IMPL_CONFIGREADER_H */
