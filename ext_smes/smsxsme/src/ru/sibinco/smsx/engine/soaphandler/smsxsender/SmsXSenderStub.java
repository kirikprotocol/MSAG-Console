/**
 * SmsXSenderSoapBindingStub.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.smsxsender;

import org.apache.axis.AxisFault;

import javax.xml.rpc.Service;
import java.rmi.RemoteException;
import java.net.URL;

public class SmsXSenderStub extends org.apache.axis.client.Stub implements SmsXSender {
  private java.util.Vector cachedSerClasses = new java.util.Vector();
  private java.util.Vector cachedSerQNames = new java.util.Vector();
  private java.util.Vector cachedSerFactories = new java.util.Vector();
  private java.util.Vector cachedDeserFactories = new java.util.Vector();

  static org.apache.axis.description.OperationDesc [] _operations;

  static {
    _operations = new org.apache.axis.description.OperationDesc[2];
    _initOperationDesc1();
  }

  private static void _initOperationDesc1(){
    org.apache.axis.description.OperationDesc oper;
    org.apache.axis.description.ParameterDesc param;
    oper = new org.apache.axis.description.OperationDesc();
    oper.setName("sendSms");
    param = new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "MSISDN"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false);
    oper.addParameter(param);
    param = new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "Message"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false);
    oper.addParameter(param);
    param = new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXExpress"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false);
    oper.addParameter(param);
    param = new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXSecret"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false);
    oper.addParameter(param);
    param = new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXCalendar"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false);
    oper.addParameter(param);
    param = new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXCalendarTimeUTC"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "long"), long.class, false, false);
    oper.addParameter(param);
    param = new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXAdvertising"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false);
    oper.addParameter(param);
    oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
    oper.setReturnClass(SmsXSenderResponse.class);
    oper.setReturnQName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "sendSmsReturn"));
    oper.setStyle(org.apache.axis.constants.Style.WRAPPED);
    oper.setUse(org.apache.axis.constants.Use.LITERAL);
    _operations[0] = oper;

    oper = new org.apache.axis.description.OperationDesc();
    oper.setName("checkStatus");
    param = new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXIdMessage"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false);
    oper.addParameter(param);
    oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
    oper.setReturnClass(SmsXSenderResponse.class);
    oper.setReturnQName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "checkStatusReturn"));
    oper.setStyle(org.apache.axis.constants.Style.WRAPPED);
    oper.setUse(org.apache.axis.constants.Use.LITERAL);
    _operations[1] = oper;

  }

  public SmsXSenderStub() throws org.apache.axis.AxisFault {
    this(null);
  }

  public SmsXSenderStub(URL endpointURL, Service service) throws AxisFault {
    this(service);
    super.cachedEndpoint = endpointURL;
  }

  public SmsXSenderStub(javax.xml.rpc.Service service) throws org.apache.axis.AxisFault {
    if (service == null) {
      super.service = new org.apache.axis.client.Service();
    } else {
      super.service = service;
    }
    ((org.apache.axis.client.Service)super.service).setTypeMappingVersion("1.2");
    java.lang.Class cls;
    javax.xml.namespace.QName qName;
    javax.xml.namespace.QName qName2;
    java.lang.Class beansf = org.apache.axis.encoding.ser.BeanSerializerFactory.class;
    java.lang.Class beandf = org.apache.axis.encoding.ser.BeanDeserializerFactory.class;
    java.lang.Class enumsf = org.apache.axis.encoding.ser.EnumSerializerFactory.class;
    java.lang.Class enumdf = org.apache.axis.encoding.ser.EnumDeserializerFactory.class;
    java.lang.Class arraysf = org.apache.axis.encoding.ser.ArraySerializerFactory.class;
    java.lang.Class arraydf = org.apache.axis.encoding.ser.ArrayDeserializerFactory.class;
    java.lang.Class simplesf = org.apache.axis.encoding.ser.SimpleSerializerFactory.class;
    java.lang.Class simpledf = org.apache.axis.encoding.ser.SimpleDeserializerFactory.class;
    java.lang.Class simplelistsf = org.apache.axis.encoding.ser.SimpleListSerializerFactory.class;
    java.lang.Class simplelistdf = org.apache.axis.encoding.ser.SimpleListDeserializerFactory.class;
    qName = new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse");
    cachedSerQNames.add(qName);
    cls = SmsXSenderResponse.class;
    cachedSerClasses.add(cls);
    cachedSerFactories.add(beansf);
    cachedDeserFactories.add(beandf);

  }

  protected org.apache.axis.client.Call createCall() throws java.rmi.RemoteException {
    try {
      org.apache.axis.client.Call _call = super._createCall();
      if (super.maintainSessionSet) {
        _call.setMaintainSession(super.maintainSession);
      }
      if (super.cachedUsername != null) {
        _call.setUsername(super.cachedUsername);
      }
      if (super.cachedPassword != null) {
        _call.setPassword(super.cachedPassword);
      }
      if (super.cachedEndpoint != null) {
        _call.setTargetEndpointAddress(super.cachedEndpoint);
      }
      if (super.cachedTimeout != null) {
        _call.setTimeout(super.cachedTimeout);
      }
      if (super.cachedPortName != null) {
        _call.setPortName(super.cachedPortName);
      }
      java.util.Enumeration keys = super.cachedProperties.keys();
      while (keys.hasMoreElements()) {
        java.lang.String key = (java.lang.String) keys.nextElement();
        _call.setProperty(key, super.cachedProperties.get(key));
      }
      // All the type mapping information is registered
      // when the first call is made.
      // The type mapping information is actually registered in
      // the TypeMappingRegistry of the service, which
      // is the reason why registration is only needed for the first call.
      synchronized (this) {
        if (firstCall()) {
          // must set encoding style before registering serializers
          _call.setEncodingStyle(null);
          for (int i = 0; i < cachedSerFactories.size(); ++i) {
            java.lang.Class cls = (java.lang.Class) cachedSerClasses.get(i);
            javax.xml.namespace.QName qName =
              (javax.xml.namespace.QName) cachedSerQNames.get(i);
            java.lang.Object x = cachedSerFactories.get(i);
            if (x instanceof Class) {
              java.lang.Class sf = (java.lang.Class)
                cachedSerFactories.get(i);
              java.lang.Class df = (java.lang.Class)
                cachedDeserFactories.get(i);
              _call.registerTypeMapping(cls, qName, sf, df, false);
            }
            else if (x instanceof javax.xml.rpc.encoding.SerializerFactory) {
              org.apache.axis.encoding.SerializerFactory sf = (org.apache.axis.encoding.SerializerFactory)
                cachedSerFactories.get(i);
              org.apache.axis.encoding.DeserializerFactory df = (org.apache.axis.encoding.DeserializerFactory)
                cachedDeserFactories.get(i);
              _call.registerTypeMapping(cls, qName, sf, df, false);
            }
          }
        }
      }
      return _call;
    }
    catch (java.lang.Throwable _t) {
      throw new org.apache.axis.AxisFault("Failure trying to get the Call object", _t);
    }
  }

  public SmsXSenderResponse sendSms(java.lang.String MSISDN, java.lang.String message, boolean SMSXExpress, boolean SMSXSecret, boolean SMSXCalendar, long SMSXCalendarTimeUTC, boolean SMSXAdvertising) throws java.rmi.RemoteException {
    if (super.cachedEndpoint == null) {
      throw new org.apache.axis.NoEndPointException();
    }
    org.apache.axis.client.Call _call = createCall();
    _call.setOperation(_operations[0]);
    _call.setUseSOAPAction(true);
    _call.setSOAPActionURI("");
    _call.setEncodingStyle(null);
    _call.setProperty(org.apache.axis.client.Call.SEND_TYPE_ATTR, Boolean.FALSE);
    _call.setProperty(org.apache.axis.AxisEngine.PROP_DOMULTIREFS, Boolean.FALSE);
    _call.setSOAPVersion(org.apache.axis.soap.SOAPConstants.SOAP11_CONSTANTS);
    _call.setOperationName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "sendSms"));

    setRequestHeaders(_call);
    setAttachments(_call);
    try {        java.lang.Object _resp = _call.invoke(new java.lang.Object[] {MSISDN, message, Boolean.valueOf(SMSXExpress), Boolean.valueOf(SMSXSecret), Boolean.valueOf(SMSXCalendar), new java.lang.Long(SMSXCalendarTimeUTC), Boolean.valueOf(SMSXAdvertising)});

      if (_resp instanceof java.rmi.RemoteException) {
        throw (java.rmi.RemoteException)_resp;
      }
      else {
        extractAttachments(_call);
        try {
          return (SmsXSenderResponse) _resp;
        } catch (java.lang.Exception _exception) {
          return (SmsXSenderResponse) org.apache.axis.utils.JavaUtils.convert(_resp, SmsXSenderResponse.class);
        }
      }
    } catch (org.apache.axis.AxisFault axisFaultException) {
      throw axisFaultException;
    }
  }

  public SmsXSenderResponse sendPaidSms(String oa, String da, String message, boolean SMSXExpress, boolean SMSXSecret, boolean SMSXCalendar, long SMSXCalendarTimeUTC) throws RemoteException {
    if (super.cachedEndpoint == null) {
      throw new org.apache.axis.NoEndPointException();
    }
    org.apache.axis.client.Call _call = createCall();
    _call.setOperation(_operations[0]);
    _call.setUseSOAPAction(true);
    _call.setSOAPActionURI("");
    _call.setEncodingStyle(null);
    _call.setProperty(org.apache.axis.client.Call.SEND_TYPE_ATTR, Boolean.FALSE);
    _call.setProperty(org.apache.axis.AxisEngine.PROP_DOMULTIREFS, Boolean.FALSE);
    _call.setSOAPVersion(org.apache.axis.soap.SOAPConstants.SOAP11_CONSTANTS);
    _call.setOperationName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "sendPaidSms"));

    setRequestHeaders(_call);
    setAttachments(_call);
    try {        java.lang.Object _resp = _call.invoke(new java.lang.Object[] {oa, da, message, Boolean.valueOf(SMSXExpress), Boolean.valueOf(SMSXSecret), Boolean.valueOf(SMSXCalendar), new java.lang.Long(SMSXCalendarTimeUTC)});

      if (_resp instanceof java.rmi.RemoteException) {
        throw (java.rmi.RemoteException)_resp;
      }
      else {
        extractAttachments(_call);
        try {
          return (SmsXSenderResponse) _resp;
        } catch (java.lang.Exception _exception) {
          return (SmsXSenderResponse) org.apache.axis.utils.JavaUtils.convert(_resp, SmsXSenderResponse.class);
        }
      }
    } catch (org.apache.axis.AxisFault axisFaultException) {
      throw axisFaultException;
    }
  }

  public int sendSysSms(String oa, String da, String message) throws RemoteException {
    return 0;
  }

  public SmsXSenderResponse checkStatus(java.lang.String SMSXIdMessage) throws java.rmi.RemoteException {
    if (super.cachedEndpoint == null) {
      throw new org.apache.axis.NoEndPointException();
    }
    org.apache.axis.client.Call _call = createCall();
    _call.setOperation(_operations[1]);
    _call.setUseSOAPAction(true);
    _call.setSOAPActionURI("");
    _call.setEncodingStyle(null);
    _call.setProperty(org.apache.axis.client.Call.SEND_TYPE_ATTR, Boolean.FALSE);
    _call.setProperty(org.apache.axis.AxisEngine.PROP_DOMULTIREFS, Boolean.FALSE);
    _call.setSOAPVersion(org.apache.axis.soap.SOAPConstants.SOAP11_CONSTANTS);
    _call.setOperationName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "checkStatus"));

    setRequestHeaders(_call);
    setAttachments(_call);
    try {        java.lang.Object _resp = _call.invoke(new java.lang.Object[] {SMSXIdMessage});

      if (_resp instanceof java.rmi.RemoteException) {
        throw (java.rmi.RemoteException)_resp;
      }
      else {
        extractAttachments(_call);
        try {
          return (SmsXSenderResponse) _resp;
        } catch (java.lang.Exception _exception) {
          return (SmsXSenderResponse) org.apache.axis.utils.JavaUtils.convert(_resp, SmsXSenderResponse.class);
        }
      }
    } catch (org.apache.axis.AxisFault axisFaultException) {
      throw axisFaultException;
    }
  }

  public SmsXSenderResponse batchSecret(String oa, String message, boolean express) throws RemoteException {
    return null;
  }

  public SmsXSenderResponse checkBatchSecretStatus(String msgId) throws RemoteException {
    return null;
  }

}
