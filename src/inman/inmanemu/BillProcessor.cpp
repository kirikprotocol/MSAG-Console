#include "inman/inmanemu/BillProcessor.h"
#include "inman/inmanemu/XMLHandlers.h"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>


namespace inmanemu { namespace processor {

using namespace inmanemu::util;
using namespace xercesc;

void BillProcessor::init()
{
    logger = Logger::getInstance("inmanemu.proc");

    smsc_log_info(logger,"Start initializing billing processor...");

    int errorCount = 0;

    try
    {
        XMLPlatformUtils::Initialize("ru_RU.KOI8-R");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());
        throw Exception("Error during initialization XMLPlatform: %s", msg.localForm());
    }


    SAXParser parser;

    XMLBasicHandler handler(this);

    try
    {
        //parser.setValidationScheme(SAXParser::Val_Always);
        //parser.setDoSchema(true);
        //parser.setValidationSchemaFullChecking(true);
        //parser.setDoNamespaces(true);

        //parser.setValidateAnnotations(false);   

        parser.setValidationConstraintFatal(true);

        parser.setDocumentHandler(&handler);
        parser.setErrorHandler(&handler);


        parser.parse("billing.xml");

        errorCount = parser.getErrorCount();
    }
    catch (const OutOfMemoryException&)
    {
        //smsc_log_error(logger,"Terminate parsing config: XMLPlatform: OutOfMemoryException");
        throw Exception("Terminate parsing billing file: XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        //smsc_log_error(logger,"Terminate parsing config: An error occurred. Error: %s", msg.localForm());
        throw Exception("Terminate parsing billing file: An error occurred. Error: %s", msg.localForm());
    }

    if (errorCount > 0) throw Exception("Terminate parsing billing file Unknown error");
    smsc_log_info(logger,"Billing processor initialized");
}


void BillProcessor::commit(int BillId)
{
    Address * abonent = BillIdDataHash.GetPtr(BillId);
    if (!abonent) 
    {
        smsc_log_error(logger, "Cannot commit billId %d. Details: Cannot find abonent for billId.", BillId);
        return;
    }

    Account * account = AccountsHash.GetPtr(*abonent);

    if (!account)
    {
         smsc_log_error(logger, "Cannot commit billId %d. Details: Cannot find billing account for abonent '%s'",
                        BillId, abonent->toString().c_str());
    } else
    {
      account->amount = account->amount - account->charged;
  
      smsc_log_error(logger, "%d commited for %s abonent (money = %d)",
                     account->charged, abonent->toString().c_str(), account->amount);
      account->charged = 0;
    }
    BillIdDataHash.Delete(BillId);
}

void BillProcessor::rollback(int BillId)
{
    Address * abonent = BillIdDataHash.GetPtr(BillId);
    if (!abonent) 
    {
        smsc_log_error(logger, "Cannot rollback billId %d. Details: Cannot find abonent for billId.", BillId);
        return;
    }

    Account * account = AccountsHash.GetPtr(*abonent);

    if (!account)
    {
      smsc_log_error(logger, "Cannot rollback billId %d. Details: Cannot find billing account for abonent '%s'",
                     BillId, abonent->toString().c_str());
    } else
    {
      smsc_log_error(logger, "Rollback %d for %s abonent (money = %d)",
                     account->charged, abonent->toString().c_str(), account->amount);
      account->charged = 0;
    }
    BillIdDataHash.Delete(BillId);
}


bool BillProcessor::charge(MatrixKey& key, Address& abonent, int BillId)
{
    int * pricePtr = BillRecordsHash.GetPtr(key);
    if (!pricePtr) 
    {
        smsc_log_error(logger, "Cannot charge billId %d. Details: Cannot find billing record for service number '%d'",
                       BillId, key.serviceNumber);
        return false;
    }

    Account * account = AccountsHash.GetPtr(abonent);
    if (!account)
    {
         smsc_log_error(logger, "Cannot charge billId %d. Details: Cannot find billing account for abonent '%s'",
                        BillId, abonent.toString().c_str());
         return false;
    }

    if (BillIdDataHash.GetPtr(BillId))
      BillIdDataHash.Delete(BillId);
    BillIdDataHash.Insert(BillId, abonent);


    if (account->amount < (*pricePtr)) 
    {
        smsc_log_error(logger, "Cannot charge %d for %s abonent (money = %d). Delails - not enough money",
                       (*pricePtr),  abonent.toString().c_str(), account->amount);
        return false;
    } 

    account->charged = (*pricePtr);
    smsc_log_error(logger, "%d successfully charged for %s abonent (money = %d, Bill=%d)",
                   (*pricePtr), abonent.toString().c_str(), account->amount, BillId);
    return true;
}


void BillProcessor::RegisterAccount(Address& abonentAddr, int money)
{
    AccountsHash.Insert(abonentAddr,money);
    smsc_log_debug(logger,"Billing processor: abonent %s registered",abonentAddr.toString().c_str());
}

void BillProcessor::RegisterBillRecord(MatrixKey& key, int price)
{
    BillRecordsHash.Insert(key, price);
    smsc_log_debug(logger,"Billing processor: service (number=%d) registered",key.serviceNumber);
}




BillProcessor::~BillProcessor()
{
    XMLPlatformUtils::Terminate();
}


}}
