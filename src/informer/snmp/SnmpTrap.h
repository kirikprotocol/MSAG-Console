#ifndef _INFORMER_SNMPTRAP_H
#define _INFORMER_SNMPTRAP_H

namespace eyeline {
namespace informer {

struct SnmpTrap
{

    typedef enum {
        TYPE_CONNECT = 1
    } Type;


    typedef enum {
        SEV_CLEAR = 1,
        SEV_NORMAL = 2,
        SEV_MINOR = 3,
        SEV_MAJOR = 4,
        SEV_CRITICAL = 5
    } Severity;

    Type         type;
    Severity     severity;
    std::string  category;
    std::string  objid;
    std::string  message;

    bool isValid() const {
        if (type != TYPE_CONNECT) return false;
        if (severity < SEV_CLEAR || severity > SEV_CRITICAL) return false;
        if (!category.size() ||
            !objid.size() ||
            !message.size()) return false;
        return true;
    }

    // print trap into buffer in human-readable format and return that buffer
    char* toString( char* buf, size_t bufsize ) const {
        snprintf(buf,bufsize,"%s %s %s %s %s",
                 typeToString(type),
                 severityToString(severity),
                 category.c_str(),
                 objid.c_str(),
                 message.c_str() );
        return buf;
    }


    static const char* typeToString( Type t ) {
        switch (t) {
        case TYPE_CONNECT: return "conn";
        default: return "???";
        }
    }
    static const char* severityToString( Severity s ) {
        switch (s) {
        case SEV_CLEAR:  return "clear";
        case SEV_NORMAL: return "norm";
        case SEV_MINOR:  return "minor";
        case SEV_MAJOR:  return "major";
        case SEV_CRITICAL: return "crit";
        default: return "???";
        }
    }
};

// function declarations
// int send_informerConnectAlert_trap( SnmpTrap& trap );

}
}

#endif /* INFORMERSNMP_H */
