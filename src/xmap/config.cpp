#include "config.h"

#include <logger/Logger.h>

#include <stdio.h>
#include <time.h>
 
#include <ctype.h>
#include <algorithm>
#include <signal.h>
 
#include <iostream>
#include <thread.h>
#include <unistd.h>
#include <core/synchronization/Mutex.hpp>

extern smsc::logger::Logger *logger;

using namespace smsc::core::synchronization;
using smsc::core::synchronization::MutexGuard;

ConfigXml::ConfigXml(){};
ConfigXml::~ConfigXml(){};

int  ConfigXml::readTimeouts(DOMElement *docElem)
{

 int tms=0;
 if(!docElem)return 0;
 XmlStr _str_timeouts("timeout");
 DOMNodeList *dnl=docElem->getElementsByTagName((XMLCh*)_str_timeouts.x_str ());
 if(dnl)
 {
  if(dnl->getLength()>0)
  {
   DOMNode* dn=dnl->item(0);
   XmlStr nodemame(dn->getNodeName());
   DOMNamedNodeMap* dnm=dn->getAttributes();
   if(dnm->getLength()>0)
   {
    XmlStr attrname("val");
    DOMNode* dval=dnm->getNamedItem((XMLCh*)attrname.x_str());
    XmlStr attrval(dval->getNodeValue());
    tms=atoi(attrval.c_str());
   }


  }
 }
 return tms;
}

int ConfigXml::readMscAddr(DOMElement *docElem)
{
 int i=0;
 if(!docElem)return 0;

 XmlStr _str_timeouts("msc");
 DOMNodeList *dnl=docElem->getElementsByTagName((XMLCh*)_str_timeouts.x_str ());
    if(dnl)
 {
  if(dnl->getLength()>0)
  {

   DOMNode* dn=dnl->item(0);
   if(dn)
   {
    DOMNamedNodeMap* dnm=dn->getAttributes();

    if(dnm->getLength()>0)
    {
     XmlStr attrname("address");
     DOMNode* dval=dnm->getNamedItem((XMLCh*)attrname.x_str());
     XmlStr attrval(dval->getNodeValue());

     str_mscaddr = attrval.c_release();
     i=1;

    }
   }
  
  }


 }

 return i;
}

int ConfigXml::readSmes(DOMElement *docElem)
{
int i=0;
 if(!docElem)return 0;

 XmlStr _str_timeouts("sme");
 DOMNodeList *dnl=docElem->getElementsByTagName((XMLCh*)_str_timeouts.x_str ());
    if(dnl)
 {
  if(dnl->getLength()>0)
  {

   DOMNode* dn=dnl->item(0);
   if(dn)
   {
    sme_address=getStringValAttr(dn,"address");
    sme_sid=getStringValAttr(dn,"sid");
    sme_host=getStringValAttr(dn,"host");
    sme_port=getIntValAttr(dn,"port");
    i=1;

    xmap_trace(logger,"%s host '%s'",__func__,sme_host.c_str());
    xmap_trace(logger,"%s port %d",__func__,sme_port);
    xmap_trace(logger,"%s addr '%s'",__func__,sme_address.c_str());
    xmap_trace(logger,"%s sid  '%s'",__func__,sme_sid.c_str());
    
   }
  
  }


 }

 return i;
}
//DEL USHORT_T ConfigXml::getPipesCount()
//DEL {
//DEL  MutexGuard g(mtx);
//DEL  mtx.notify();
//DEL  return vpipes.size();
//DEL }
long ConfigXml::getMqueueLength()
{
 MutexGuard g(mtx);
 mtx.notify();
 return mqueuelen;
}
std::string ConfigXml::getStringValAttr(DOMNode*  node,const char * attr_name)
{

 std::string str="";

 XmlStr attrname(attr_name);
 if(node)
 {
  DOMNamedNodeMap* dnm=node->getAttributes();
  
  if(dnm->getLength()>0)
  {
   DOMNode* dval=dnm->getNamedItem((XMLCh*)attrname.x_str());
   XmlStr attrval(dval->getNodeValue());
   str=attrval.c_release();
  }
  
 }
 
 

 return str;
}

long ConfigXml::getIntValAttr(DOMNode* node,const char * attr_name)
{
 int result=0;

 XmlStr attrname(attr_name);
 if(node)
 {
  DOMNamedNodeMap* dnm=node->getAttributes();

  if(dnm->getLength()>0)
  {
   DOMNode* dval=dnm->getNamedItem((XMLCh*)attrname.x_str());
   ////xmap_trace(logger,"%s [0x%x]",attrname.c_str(),dval);
   if(dval)
   {
    XmlStr attrval(dval->getNodeValue());
    result=atol(attrval.c_release());
    
   }

   
  }
 }

 return result;
}

bool ConfigXml::isAutostart()
{
 MutexGuard g(mtx);
 return autostart;
}
//DEL int ConfigXml::readPipes(DOMElement *docElem)
//DEL {
//DEL  int i=0;
//DEL  if(!docElem)return 0;
//DEL 
//DEL  XmlStr _str_timeouts("pipe");
//DEL  DOMNodeList *dnl=docElem->getElementsByTagName((XMLCh*)_str_timeouts.x_str ());
//DEL  if(dnl)
//DEL  {
//DEL   if(dnl->getLength()>0)
//DEL   {
//DEL    i=1;
//DEL    for(int k=0;k<dnl->getLength();k++)
//DEL    {
//DEL    
//DEL     DOMNode* dn=dnl->item(k);
//DEL     if(dn)
//DEL     {
//DEL      cpipe pipe;
//DEL      
//DEL      pipe.originating_addr=getStringValAttr(dn,"oraddr");
//DEL      pipe.destination_addr=getStringValAttr(dn,"destaddr");
//DEL      pipe.imsi =getStringValAttr(dn,"imsi");
//DEL      pipe.random_case = (USHORT_T)getIntValAttr(dn,"random");
//DEL      pipe.message_text = getStringValAttr(dn,"text");
//DEL      pipe.sender = (getIntValAttr(dn,"sender")==1);
//DEL 
//DEL      vpipes.push_back(pipe);
//DEL      //xmap_trace(logger,"added pipe [imsi %s] [oraddr %s] sender=%d text='%s'",pipe.imsi.c_str(),pipe.originating_addr.c_str(),pipe.sender,pipe.message_text.c_str());
//DEL 
//DEL      i++;    
//DEL 
//DEL     }
//DEL     
//DEL    }
//DEL    
//DEL 
//DEL   }
//DEL  }
//DEL 
//DEL  return i;
//DEL }
//DEL cpipe ConfigXml::getPipeByMSISDN(std::string & msisdn)
//DEL {
//DEL  MutexGuard g(mtx);
//DEL  cpipe p;
//DEL  p.imsi=p.destination_addr=p.message_text=p.originating_addr="";
//DEL 
//DEL  ////xmap_trace(logger,"ConfigXml::getPipeByMSISDN(%s) size==%d",msisdn.c_str(),vpipes.size());
//DEL 
//DEL  for(int i=0;i<vpipes.size();i++)
//DEL  {
//DEL   if(vpipes[i].originating_addr.compare(msisdn.c_str())==0)
//DEL   {
//DEL    mtx.notify();
//DEL    return vpipes[i];
//DEL   }
//DEL  }
//DEL  mtx.notify();
//DEL  return p;
//DEL }


int ConfigXml::readXMLConfig(const char* filename)
{
 int result =0;
 initXerces();
 {
  DOMTreeReader reader;

  try{
   DOMDocument *doc = reader.read(filename);
   DOMElement *docElem = doc->getDocumentElement();

   
   if (docElem)
   {
    timeout=readTimeouts(docElem);
    smsc_ssn=getIntValAttr(docElem,"smsc_ssn");
    ussd_ssn=getIntValAttr(docElem,"ussd_ssn");
    mqueuelen=getIntValAttr(docElem,"mqueuelen");
    autostart=(getIntValAttr(docElem,"autostart")==1);

    //xmap_trace(logger,"timeout  is %d",timeout);
    //xmap_trace(logger,"smsc ssn is %d",smsc_ssn);
    //xmap_trace(logger,"ussd ssn is %d",ussd_ssn);
    //xmap_trace(logger,"message queue length is %d",mqueuelen);
    
    if(readMscAddr(docElem))
    {
     //xmap_trace(logger,"MSCADDR=%s",str_mscaddr.c_str());
    }
    //if (int k=readPipes(docElem)){
     //xmap_trace(logger,"pipes count %d",vpipes.size());
    //}
    readSmes(docElem);

    result=1;

   }
   else
   {
    //xmap_trace(logger,"error!: in xmap.xml");
    result=0;
   }
    
    
  }
  catch(...)
  {
   //xmap_trace(logger,"error!: xmap.xml not found");
   result=0;
  }

 }
 //abort();
 TerminateXerces();

 return result;
}

std::string ConfigXml::getMscAddr()
{
 MutexGuard g(mtx);
 mtx.notify();
 return str_mscaddr;
}

std::string ConfigXml::getSmeAddress()
{
 return sme_address;
}
std::string ConfigXml::getSmeSid()
{
 return sme_sid;
}

int ConfigXml::getSmePort()
{
 return sme_port;
}

std::string ConfigXml::getSmeHost()
{
 return sme_host;
}
////////////
//cpipe
 