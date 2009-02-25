#ifndef _SCAG_PVSS_COMMON_SCOPETYPE_H
#define _SCAG_PVSS_COMMON_SCOPETYPE_H

namespace scag2 {
namespace pvss {

enum ScopeType
{
        ABONENT = 0x01,
        OPERATOR = 0x02,
        PROVIDER = 0x03,
        SERVICE = 0x04
};

inline const char* scopeTypeToString( ScopeType t ) {
    switch ( t ) {
#define CASETOSTRING(x) case x: return #x
    CASETOSTRING(ABONENT);
    CASETOSTRING(OPERATOR);
    CASETOSTRING(PROVIDER);
    CASETOSTRING(SERVICE);
#undef CASETOSTRING
    default: return "UNKNOWN";
    }
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_COMMON_SCOPETYPE_H */
