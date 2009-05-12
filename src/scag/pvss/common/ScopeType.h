#ifndef _SCAG_PVSS_COMMON_SCOPETYPE_H
#define _SCAG_PVSS_COMMON_SCOPETYPE_H

namespace scag2 {
namespace pvss {

enum ScopeType
{
    // NOTE: 0x00 is reserved for UNKNOWN
        SCOPE_ABONENT = 0x01,
        SCOPE_OPERATOR = 0x02,
        SCOPE_PROVIDER = 0x03,
        SCOPE_SERVICE = 0x04
};

inline const char* scopeTypeToString( ScopeType t ) {
    switch ( t ) {
#define CASETOSTRING(x) case SCOPE_##x: return #x
    CASETOSTRING(ABONENT);
    CASETOSTRING(OPERATOR);
    CASETOSTRING(PROVIDER);
    CASETOSTRING(SERVICE);
#undef CASETOSTRING
    default: return "UNKNOWN";
    }
};

inline ScopeType scopeTypeFromString( const char* t ) {
    if (!t) {return ScopeType(0);}
    else if ( !strcmp(t,"ABONENT") )  { return SCOPE_ABONENT; }
    else if ( !strcmp(t,"OPERATOR") ) { return SCOPE_OPERATOR; }
    else if ( !strcmp(t,"PROVIDER") ) { return SCOPE_PROVIDER; }
    else if ( !strcmp(t,"SERVICE") )  { return SCOPE_SERVICE; }
    else { return ScopeType(0); }
}

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_COMMON_SCOPETYPE_H */
