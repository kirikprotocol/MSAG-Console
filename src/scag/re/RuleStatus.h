#ifndef __SCAG_RE_RULESTATUS__
#define __SCAG_RE_RULESTATUS__

namespace scag { namespace re {

enum StatusEnum
{
    STATUS_OK,
    STATUS_FAILED,
    STATUS_REDIRECT,
    STATUS_LONG_CALL
};

class PostProcessAction
{
public:
    PostProcessAction* next;
    PostProcessAction() : next(NULL) {};
    virtual void run() = 0;   
    virtual ~PostProcessAction() {};
};

struct RuleStatus
{
    int result;
    StatusEnum status;
    bool temporal;
    PostProcessAction *actions, *actionsTail;
    //RuleStatus() : result(false),hasErrors(false) {};
    RuleStatus() : result(0), status(STATUS_OK),temporal(false), actions(NULL) {};
    void addAction(PostProcessAction* p)
    {
        if(actions)
            actionsTail->next = p;
        else
            actions = p;
        actionsTail = p;
    }
    void runPostProcessActions()
    {
        PostProcessAction* p = actions;
        while(p)
        {
            p->run();
            p = p->next;
        }
    }
    ~RuleStatus()
    {
        while(actions)
        {
            PostProcessAction* c = actions;
            actions = actions->next;
            delete c;
        }
    }
private:
    RuleStatus(const RuleStatus& cp) {};
    RuleStatus& operator=(const RuleStatus& cp) { return *this; };
};

}}

#endif

