#ifndef _SMSC_ADMIN_SMSCCOMMANDS2_H_
#define _SMSC_ADMIN_SMSCCOMMANDS2_H_

#include "AdminCommand2.h"


namespace scag2 { namespace admin {

//========================================================================
//================== Smsc commands =======================================
class Abstract_CommandSmsc : public AdminCommand
{
public:
  Abstract_CommandSmsc(const Command::Id id, const xercesc::DOMDocument * const doc)
        : AdminCommand(id, doc)
    {
    }

    virtual void init();
protected:
  std::string systemId;
};

class CommandUpdateSmsc : public Abstract_CommandSmsc
{
public:
  CommandUpdateSmsc(const xercesc::DOMDocument * const doc)
    : Abstract_CommandSmsc((Command::Id)CommandIds::updateSmsc, doc)
  {
  }
    virtual Response * CreateResponse( Scag * SmscApp);
};

class CommandAddSmsc : public Abstract_CommandSmsc
{
public:
  CommandAddSmsc(const xercesc::DOMDocument * const doc)
    : Abstract_CommandSmsc((Command::Id)CommandIds::addSmsc, doc)
  {
  }
    virtual Response * CreateResponse(Scag * SmscApp);
};

class CommandDeleteSmsc : public AdminCommand
{
protected:
    std::string systemId;
public:
    CommandDeleteSmsc(const xercesc::DOMDocument * const doc)
        : AdminCommand((Command::Id)CommandIds::deleteSmsc, doc)
    {
    }

    virtual Response * CreateResponse(Scag * SmscApp);
    virtual void init();
};



}}

#endif

