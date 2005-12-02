#ifndef _SMPP_PROP_H
#define _SMPP_PROP_H


 
#include <core/buffers/FastMTQueue.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/synchronization/Event.hpp>
#include <sms/sms.h>

#include <util/recoder/recode_dll.h>
#include <logger/Logger.h>
#include "propobj.h"
#include <jsapi.h>

using namespace smsc::logger;
using namespace smsc::core::synchronization;
using namespace smsc::sms;
using namespace smsc::sms::Tag;
extern smsc::logger::Logger *logger;

class JSSmpp_prop
{
public:
 /**
  * Constructor
  */
 JSSmpp_prop() : m_pProp(NULL)
 {
 };
 
 /**
  * Destructor
  */
 virtual ~JSSmpp_prop()
 {
  delete m_pProp;
  m_pProp = NULL;
 };

 /**
  * JSGetProperty - Callback for retrieving properties
  */
 static JSBool JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

 /**
  * JSSetProperty - Callback for setting properties
  */
 static JSBool JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

 /**
  * JSConstructor - Callback for when a wxCustomer object is created
  */
 static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 
 /**
  * JSDestructor - Callback for when a wxCustomer object is destroyed
  */
 static void JSDestructor(JSContext *cx, JSObject *obj);

 /**
  * JSInit - Create a prototype for wxCustomer
  */
 static JSObject* JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL);


 static JSBool setStrProp(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool setIntProp(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool getStrProp(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
 static JSBool getIntProp(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

 static JSClass Prop_class;

 void setProp(Prop *pr)
 {
  m_pProp  = pr; 
 };
    
 Prop* getProp() 
 {
  return m_pProp; 
 };

private:

 Prop *m_pProp;
 static JSFunctionSpec Prop_methods[];


};


#endif