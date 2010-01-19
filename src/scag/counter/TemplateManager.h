#ifndef _SCAG_COUNTER_TEMPLATEMANAGER_H
#define _SCAG_COUNTER_TEMPLATEMANAGER_H

namespace scag2 {
namespace counter {

class Counter;

class TemplateManager
{
public:
    virtual ~TemplateManager() {}
    virtual Counter* createCounter( const std::string& templname,
                                    const char* templid,
                                    unsigned lifetimeSeconds ) = 0;
};

}
}

#endif /* !_SCAG_COUNTER_TEMPLATEMANAGER_H */
