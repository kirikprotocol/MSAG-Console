#include "smpp_prop.h"

#include <sms/sms.h>

using namespace smsc::sms;
using namespace smsc::sms::Tag;




JSFunctionSpec JSSmpp_prop::Prop_methods[] = 
{
 {"SetIntProp",setIntProp,1,0,0},
 {"SetStrProp",setStrProp,1,0,0},
 {"GetIntProp",getIntProp,1,0,0},
 {"GetStrProp",getStrProp,1,0,0},
    { 0, 0, 0, 0, 0 }
};

JSClass JSSmpp_prop::Prop_class = 
{ 
 "PropContainer", JSCLASS_HAS_PRIVATE, JS_PropertyStub, JS_PropertyStub,
 JSSmpp_prop::JSGetProperty, JSSmpp_prop::JSSetProperty,
        JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JSSmpp_prop::JSDestructor
};

JSBool JSSmpp_prop::JSGetProperty(JSContext *cx, JSObject *obj, jsval id,jsval *vp)
{
   return JS_TRUE;
}

JSBool JSSmpp_prop::JSSetProperty(JSContext *cx, JSObject *obj, jsval id,jsval *vp)
{
    return JS_TRUE;
}

JSBool JSSmpp_prop::JSConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
 JSSmpp_prop *priv = new JSSmpp_prop();
 priv->setProp(new Prop());

 JS_SetPrivate(cx, obj, (void *) priv);

 return JS_TRUE;
}

void JSSmpp_prop::JSDestructor(JSContext *cx, JSObject *obj)
{
 JSSmpp_prop *priv = (JSSmpp_prop*) JS_GetPrivate(cx, obj);
 delete priv;
 priv = NULL;
}

JSObject *JSSmpp_prop::JSInit(JSContext *cx, JSObject *obj, JSObject *proto)
{
    JSObject *newProtoObj = JS_InitClass(cx, obj, proto, &Prop_class, 
            JSSmpp_prop::JSConstructor, 0,
           NULL, JSSmpp_prop::Prop_methods,
           NULL, NULL);

// JS_DefineProperties(cx, newProtoObj, JSSmpp_prop::Pipe_properties);


  return newProtoObj;
}

JSBool JSSmpp_prop::setStrProp(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  JSSmpp_prop *p = (JSSmpp_prop*) JS_GetPrivate(cx, obj);

 if(argc==2)
 {
    std::string p_val= JS_GetStringBytes(JS_ValueToString(cx, argv[1]));
    int p_index = JSVAL_TO_INT(argv[0]);
    p->getProp()->setStrProp((uint32_t)p_index,p_val);
 }
  
  return JS_TRUE;
}
JSBool JSSmpp_prop::setIntProp(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
return JS_TRUE;
}
JSBool JSSmpp_prop::getStrProp(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{

  JSSmpp_prop *p = (JSSmpp_prop*) JS_GetPrivate(cx, obj);

 if(argc==1)
 {
    int p_index = JSVAL_TO_INT(argv[0]);

 //   smsc_log_debug(logger,"PROP str is :%s",p->getProp()->getStrProp(p_index).c_str());
    //*rval = STRING_TO_JSVAL(p->getProp()->getStrProp(p_index).c_str());
    
   JSString *pstr=JS_NewStringCopyZ(cx,/*p->getProp()->getStrProp(p_index).c_str()*/"testtestest");

   *rval=STRING_TO_JSVAL(&pstr);

 }
 
 return JS_TRUE;
}

JSBool JSSmpp_prop::getIntProp(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
return JS_TRUE;
}

