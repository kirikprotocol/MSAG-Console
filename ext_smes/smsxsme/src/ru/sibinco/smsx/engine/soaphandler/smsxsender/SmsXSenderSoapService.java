/**
 * SmsXSenderSoapBindingSkeleton.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.smsxsender;

public class SmsXSenderSoapService implements SmsXSender, org.apache.axis.wsdl.Skeleton {
    private SmsXSender impl;
    private static java.util.Map _myOperations = new java.util.Hashtable();
    private static java.util.Collection _myOperationsList = new java.util.ArrayList();

    /**
    * Returns List of OperationDesc objects with this name
    */
    public static java.util.List getOperationDescByName(java.lang.String methodName) {
        return (java.util.List)_myOperations.get(methodName);
    }

    /**
    * Returns Collection of OperationDescs
    */
    public static java.util.Collection getOperationDescs() {
        return _myOperationsList;
    }

    static {
        org.apache.axis.description.OperationDesc _oper;
        org.apache.axis.description.FaultDesc _fault;
        org.apache.axis.description.ParameterDesc [] _params;
        _params = new org.apache.axis.description.ParameterDesc [] {
            new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "MSISDN"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false), 
            new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "Message"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false), 
            new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "SMSXExpress"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false), 
            new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "SMSXSecret"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false), 
            new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "SMSXCalendar"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false), 
            new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "SMSXCalendarTimeUTC"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "long"), long.class, false, false), 
            new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "SMSXAdvertising"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false), 
        };
        _oper = new org.apache.axis.description.OperationDesc("sendSms", _params, new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "sendSmsReturn"));
        _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
        _oper.setElementQName(new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "sendSms"));
        _oper.setSoapAction("");
        _myOperationsList.add(_oper);
        if (_myOperations.get("sendSms") == null) {
            _myOperations.put("sendSms", new java.util.ArrayList());
        }
        ((java.util.List)_myOperations.get("sendSms")).add(_oper);
        _params = new org.apache.axis.description.ParameterDesc [] {
            new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "SMSXIdMessage"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false), 
        };
        _oper = new org.apache.axis.description.OperationDesc("checkStatus", _params, new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "checkStatusReturn"));
        _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
        _oper.setElementQName(new javax.xml.namespace.QName("http://smsxsend.sibinco.ru", "checkStatus"));
        _oper.setSoapAction("");
        _myOperationsList.add(_oper);
        if (_myOperations.get("checkStatus") == null) {
            _myOperations.put("checkStatus", new java.util.ArrayList());
        }
        ((java.util.List)_myOperations.get("checkStatus")).add(_oper);
    }

    public SmsXSenderSoapService() {
        this.impl = SmsXSenderFactory.createSmsXSenderHandler();
    }

    public SmsXSenderSoapService(SmsXSender impl) {
        this.impl = impl;
    }
    public SmsXSenderResponse sendSms(java.lang.String MSISDN, java.lang.String message, boolean SMSXExpress, boolean SMSXSecret, boolean SMSXCalendar, long SMSXCalendarTimeUTC, boolean SMSXAdvertising) throws java.rmi.RemoteException
    {
        SmsXSenderResponse ret = impl.sendSms(MSISDN, message, SMSXExpress, SMSXSecret, SMSXCalendar, SMSXCalendarTimeUTC, SMSXAdvertising);
        return ret;
    }

    public SmsXSenderResponse checkStatus(java.lang.String SMSXIdMessage) throws java.rmi.RemoteException
    {
        SmsXSenderResponse ret = impl.checkStatus(SMSXIdMessage);
        return ret;
    }

}
