
#include "WSmeComponent.h"

namespace smsc { namespace wsme 
{

using namespace smsc::core::buffers;

const char* ARGUMENT_NAME_MSISDN = "msisdn";
const char* ARGUMENT_NAME_MASK   = "mask";
const char* ARGUMENT_NAME_LANG   = "lang";
const char* ARGUMENT_NAME_ID     = "id";
const char* ARGUMENT_NAME_AD     = "ad";

WSmeComponent::WSmeComponent(WSmeAdmin& _admin)
    : log(Logger::getInstance("smsc.wsme.WSmeComponent")),
        admin(_admin)
{
    Parameters visitor_params;
    visitor_params[ARGUMENT_NAME_MSISDN] = Parameter(ARGUMENT_NAME_MSISDN, StringType);
    Method add_visitor((unsigned)addVisitorMethod, "add_visitor",
                       visitor_params, StringType);
    Method remove_visitor((unsigned)removeVisitorMethod, "remove_visitor",
                       visitor_params, StringType);

    Parameters add_lang_params;
    add_lang_params[ARGUMENT_NAME_MASK] = Parameter(ARGUMENT_NAME_MASK, StringType);
    add_lang_params[ARGUMENT_NAME_LANG] = Parameter(ARGUMENT_NAME_LANG, StringType);
    Method add_lang((unsigned)addLangMethod, "add_lang",
                       add_lang_params, StringType);

    Parameters rem_lang_params;
    rem_lang_params[ARGUMENT_NAME_MASK] = Parameter(ARGUMENT_NAME_MASK, StringType);
    Method remove_lang((unsigned)removeLangMethod, "remove_lang",
                       rem_lang_params, StringType);

    Parameters add_ad_params;
    add_ad_params[ARGUMENT_NAME_ID]   = Parameter(ARGUMENT_NAME_ID, LongType);
    add_ad_params[ARGUMENT_NAME_LANG] = Parameter(ARGUMENT_NAME_LANG, StringType);
    add_ad_params[ARGUMENT_NAME_AD]   = Parameter(ARGUMENT_NAME_AD, StringType);
    Method add_ad((unsigned)addAdMethod, "add_ad",
                       add_ad_params, StringType);

    Parameters rem_ad_params;
    rem_ad_params[ARGUMENT_NAME_ID]   = Parameter(ARGUMENT_NAME_ID, LongType);
    rem_ad_params[ARGUMENT_NAME_LANG] = Parameter(ARGUMENT_NAME_LANG, StringType);
    Method remove_ad((unsigned)removeAdMethod, "remove_ad",
                       rem_ad_params, StringType);

    methods[add_visitor.getName()]    = add_visitor;
    methods[remove_visitor.getName()] = remove_visitor;
    methods[add_lang.getName()]       = add_lang;
    methods[remove_lang.getName()]    = remove_lang;
    methods[add_ad.getName()]         = add_ad;
    methods[remove_ad.getName()]      = remove_ad;
}

WSmeComponent::~WSmeComponent()
{
}

Variant WSmeComponent::call(const Method& method, const Arguments& args)
    throw (AdminException)
{
    try 
    {
        smsc_log_debug(log, "call \"%s\"", method.getName());
        
        switch (method.getId())
        {
        case addVisitorMethod:
            smsc_log_debug(log, "addVisitorMethod processing...");
            addVisitor(args);
            smsc_log_debug(log, "addVisitorMethod processed.");
            break;
        case removeVisitorMethod:
            smsc_log_debug(log, "removeVisitorMethod processing...");
            removeVisitor(args);
            smsc_log_debug(log, "removeVisitorMethod processed.");
            break;
        case addLangMethod:
            smsc_log_debug(log, "addLangMethod processing...");
            addLang(args);
            smsc_log_debug(log, "addLangMethod processed.");
            break;
        case removeLangMethod:
            smsc_log_debug(log, "removeLangMethod processing...");
            removeLang(args);
            smsc_log_debug(log, "removeLangMethod processed.");
            break;
        case addAdMethod:
            smsc_log_debug(log, "addAdMethod processing...");
            addAd(args);
            smsc_log_debug(log, "addAdMethod processed.");
            break;
        case removeAdMethod:
            smsc_log_debug(log, "removeAdMethod processing...");
            removeAd(args);
            smsc_log_debug(log, "removeAdMethod processed.");
            break;

        default:
            smsc_log_debug(log, "unknown method \"%s\" [%u]", method.getName(), method.getId());
            throw AdminException("Unknown method \"%s\"", method.getName());
        }

        return Variant("");
    }
    catch (ProcessException &exc) {
        smsc_log_debug(log, "ProcessException: %s", exc.what());
        throw AdminException(exc.what());
    }
    catch (AdminException &e) {
        smsc_log_debug(log, "AdminException: %s", e.what());
        throw e;
    }
    catch (...) {
        smsc_log_debug(log, "... Exception");
        throw AdminException("Unknown exception catched during call");
    }
}

void WSmeComponent::error(const char* method, const char* param)
    //throw (ProcessException)
{
    ProcessException exc("Parameter '%s' missed or invalid "
                         "for WSmeComponent::%s()", param, method);
    smsc_log_error(log, exc.what());
    throw exc;
}
void WSmeComponent::addVisitor(const Arguments& args)
    //throw (ProcessException)
{
    if (!args.Exists(ARGUMENT_NAME_MSISDN)) 
        error("addVisitor()", ARGUMENT_NAME_MSISDN);
    Variant arg = args[ARGUMENT_NAME_MSISDN];
    const char* msisdn = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!msisdn) 
        error("addVisitor()", ARGUMENT_NAME_MSISDN);
    
    admin.addVisitor(msisdn); 
}
void WSmeComponent::removeVisitor(const Arguments& args)
    //throw (ProcessException)            
{
    if (!args.Exists(ARGUMENT_NAME_MSISDN)) 
        error("removeVisitor()", ARGUMENT_NAME_MSISDN);
    Variant arg = args[ARGUMENT_NAME_MSISDN];
    const char* msisdn = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!msisdn) 
        error("removeVisitor()", ARGUMENT_NAME_MSISDN);
     
    admin.removeVisitor(msisdn);
}
void WSmeComponent::addLang(const Arguments& args)
    //throw (ProcessException)
{
    if (!args.Exists(ARGUMENT_NAME_MASK)) 
        error("addLang()", ARGUMENT_NAME_MASK);
    if (!args.Exists(ARGUMENT_NAME_LANG)) 
        error("addLang()", ARGUMENT_NAME_LANG);
    Variant arg = args[ARGUMENT_NAME_MASK];
    const char* mask = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!mask) 
        error("addLang()", ARGUMENT_NAME_MASK);
    arg = args[ARGUMENT_NAME_LANG];
    const char* lang = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!lang) 
        error("addLang()", ARGUMENT_NAME_LANG);
    
    admin.addLang(mask, lang);
}
void WSmeComponent::removeLang(const Arguments& args)
    //throw (ProcessException)
{
    if (!args.Exists(ARGUMENT_NAME_MASK)) 
        error("removeLang()", ARGUMENT_NAME_MASK);
    Variant arg = args[ARGUMENT_NAME_MASK];
    const char* mask = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!mask) 
        error("removeLang()", ARGUMENT_NAME_MASK);
    
    admin.removeLang(mask);
}
void WSmeComponent::addAd(const Arguments& args)
    //throw (ProcessException)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("addAd()", ARGUMENT_NAME_ID);
    if (!args.Exists(ARGUMENT_NAME_LANG)) 
        error("addAd()", ARGUMENT_NAME_LANG);
    if (!args.Exists(ARGUMENT_NAME_AD)) 
        error("addAd()", ARGUMENT_NAME_AD);

    Variant arg = args[ARGUMENT_NAME_ID];
    int id = (arg.getType() == LongType) ? arg.getLongValue():-1;
    if (id == -1)
        error("addAd()", ARGUMENT_NAME_ID);
    arg = args[ARGUMENT_NAME_LANG];
    const char* lang = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!lang) 
        error("addAd()", ARGUMENT_NAME_LANG);
    arg = args[ARGUMENT_NAME_AD];
    const char* ad = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!ad) 
        error("addAd()", ARGUMENT_NAME_AD);

    admin.addAd(id, lang, ad);
}
void WSmeComponent::removeAd(const Arguments&  args)
    //throw (ProcessException)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("removeAd()", ARGUMENT_NAME_ID);
    if (!args.Exists(ARGUMENT_NAME_LANG)) 
        error("removeAd()", ARGUMENT_NAME_LANG);

    Variant arg = args[ARGUMENT_NAME_ID];
    int id = (arg.getType() == LongType) ? arg.getLongValue():-1;
    if (id == -1)
        error("removeAd()", ARGUMENT_NAME_ID);
    arg = args[ARGUMENT_NAME_LANG];
    const char* lang = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!lang) 
        error("removeAd()", ARGUMENT_NAME_LANG);
    
    admin.removeAd(id, lang);
}

}}

