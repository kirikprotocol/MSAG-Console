/* $Id$ */

#include "CentralPersServer.h"
#include "scag/exc/SCAGExceptions.h"

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include "XMLHandlers.h"

namespace scag { namespace cpers {

using namespace scag::exceptions;
using smsc::util::Exception;
using smsc::logger::Logger;

void CentralPersServer::ParseFile(const char* _xmlFile, HandlerBase* handler)
{
    SAXParser parser;
    
    try
    {
        parser.setValidationScheme(SAXParser::Val_Always);
        parser.setDoSchema(true);
        parser.setValidationSchemaFullChecking(true);
        parser.setDoNamespaces(true);

        parser.setValidateAnnotations(false);

        parser.setValidationConstraintFatal(true);

        parser.setDocumentHandler(handler);
        parser.setErrorHandler(handler);
        parser.parse(_xmlFile);
    }
    catch (const OutOfMemoryException&)
    {
        smsc_log_error(logger,"Terminate parsing: XMLPlatform: OutOfMemoryException");
        throw Exception("XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        smsc_log_error(logger,"Terminate parsing: XMLException: %s", msg.localForm());
        throw Exception("XMLException: %s", msg.localForm());
    }
    catch (Exception& e)
    {
        smsc_log_error(logger,"Terminate parsing: %s",e.what());
        throw e;
    }
    catch (...)
    {
        smsc_log_error(logger,"Terminate parsing: unknown fatal error");
        throw Exception("unknown fatal error");
    }
}

void CentralPersServer::reloadRegions(const char* regionsFileName)
{
    MutexGuard mt(regionsReloadMutex);
    
    smsc_log_info(logger, "ReloadRegions Started");

    IntHash<RegionInfo> *hash = new IntHash<RegionInfo>();
    try{
        XMLBasicHandler handler(hash);
        ParseFile(regionsFileName, &handler);

        MutexGuard mt1(regionsMapMutex);
        delete regions;
        regions = hash;
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Regions reload was not successful");
        delete hash;
        throw;
    }

    smsc_log_info(logger, "ReloadRegions Finished");
}

CentralPersServer::CentralPersServer(const char* persHost_, int persPort_, int maxClientCount_, int timeout_, const std::string& dbPath, const std::string& dbName, uint32_t indexGrowth, const char* regionsFileName):
        PersSocketServer(persHost_, persPort_, maxClientCount_, timeout_), logger(Logger::getInstance("cpersserv")), regions(NULL)
{
    reloadRegions(regionsFileName);
        
   	if(profileInfoAllocator.Init(dbPath + '/' + dbName, indexGrowth))
        throw SCAGException("Init ProfileInfoStore failed: dbPath=%s, dbName=%s", dbPath.c_str(), dbName.c_str());
        
   	profileInfoStore.SetAllocator(&profileInfoAllocator);
   	profileInfoStore.SetChangesObserver(&profileInfoAllocator);
}

bool CentralPersServer::getRegionInfo(uint32_t id, RegionInfo& ri)
{
    MutexGuard mt(regionsMapMutex);
    RegionInfo* pri = regions->GetPtr(id);
    if(!pri) return false;
    ri = *pri;
    return true;
}

bool CentralPersServer::getProfileInfo(std::string& key, ProfileInfo& pi)
{
    return profileInfoStore.Get(AbntAddr(key.c_str()), pi);
}

bool CentralPersServer::processPacket(ConnectionContext& ctx)
{
    SerialBuffer &isb = ctx.inbuf, &osb = ctx.outbuf;

    try{
        isb.setPos(4);    
        uint8_t cmd = isb.ReadInt8();
        if(cmd == CentralPersCmd::LOGIN)
        {
            uint32_t id = isb.ReadInt32();
            std::string psw;
            isb.ReadString(psw);
            RegionInfo ri;
            return getRegionInfo(id, ri) && (ctx.authed = !strncmp(ri.passwd.c_str(), psw.c_str(), 30));
        }
        else if(!ctx.authed)
            return false;
    
        std::string key;    
        isb.ReadString(key);
        ProfileInfo profileInfo;
        TransactionInfo transactInfo, *pti;
        AbntAddr addr(key.c_str());
        profileInfoStore.Get(addr, profileInfo);
        if(pti = transactions.GetPtr(addr))
            transactInfo = *pti;
        switch(cmd)
        {
            case CentralPersCmd::GET:
/*                if(!getProfileInfo(key))
                {
                    sendCmd(ctx, isb);
                }
                else
                {
                }*/
                break;
            case CentralPersCmd::PROFILE:
                break;
            case CentralPersCmd::ACK:
                break;
            case CentralPersCmd::DONE:
                break;
            case CentralPersCmd::CHECK_OWN:
                break;
        }
    }
    catch(...)
    {
    }
    return true;
}

}}
