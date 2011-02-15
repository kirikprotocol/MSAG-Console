/**
 * SmsXSenderSoapBindingSkeleton.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.smsxsender;

import org.apache.axis.MessageContext;
import org.apache.axis.Message;
import org.apache.axis.attachments.Attachments;

import javax.xml.soap.AttachmentPart;
import javax.xml.soap.SOAPException;
import javax.activation.DataHandler;
import java.rmi.RemoteException;
import java.io.InputStream;
import java.io.IOException;
import java.util.Iterator;

public class SmsXSenderSoapService implements SmsXSender, org.apache.axis.wsdl.Skeleton {
  private SmsXSenderHandler impl;
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
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "MSISDN"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "Message"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXExpress"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXSecret"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXCalendar"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXCalendarTimeUTC"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "long"), long.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXAdvertising"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("sendSms", _params, new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "sendSmsReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "sendSms"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("sendSms") == null) {
      _myOperations.put("sendSms", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("sendSms")).add(_oper);

    _params = new org.apache.axis.description.ParameterDesc [] {
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SourceAddress"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "DestinationAddress"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "Message"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXExpress"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXSecret"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXCalendar"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXCalendarTimeUTC"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "long"), long.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("sendPaidSms", _params, new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "sendSmsReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "sendPaidSms"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("sendPaidSms") == null) {
      _myOperations.put("sendPaidSms", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("sendPaidSms")).add(_oper);

    _params = new org.apache.axis.description.ParameterDesc [] {
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXIdMessage"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("checkStatus", _params, new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "checkStatusReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "checkStatus"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("checkStatus") == null) {
      _myOperations.put("checkStatus", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("checkStatus")).add(_oper);

    _params = new org.apache.axis.description.ParameterDesc [] {
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SourceAddress"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "Message"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SMSXExpress"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("batchSecret", _params, new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "batchSecretReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "batchSecret"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("batchSecret") == null) {
      _myOperations.put("batchSecret", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("batchSecret")).add(_oper);

    _params = new org.apache.axis.description.ParameterDesc [] {
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "BatchId"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("checkBatchSecretStatus", _params, new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "checkBatchSecretStatusReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "checkBatchSecretStatus"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("checkBatchSecretStatus") == null) {
      _myOperations.put("checkBatchSecretStatus", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("checkBatchSecretStatus")).add(_oper);

    _params = new org.apache.axis.description.ParameterDesc [] {
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SourceAddress"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "DestinationAddress"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "Message"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("sendSysSms", _params, new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "sendSysSmsReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderResponse"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "sendSysSms"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("sendSysSms") == null) {
      _myOperations.put("sendSysSms", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("sendSysSms")).add(_oper);
  }

  public SmsXSenderSoapService() {
    this.impl = SmsXSenderFactory.createSmsXSenderHandler();
  }

  public SmsXSenderSoapService(SmsXSenderHandler impl) {
    this.impl = impl;
  }

  public SmsXSenderResponse sendSms(java.lang.String MSISDN, java.lang.String message, boolean SMSXExpress, boolean SMSXSecret, boolean SMSXCalendar, long SMSXCalendarTimeUTC, boolean SMSXAdvertising) throws java.rmi.RemoteException
  {
    return impl.sendSms(MSISDN, message, SMSXExpress, SMSXSecret, SMSXCalendar, SMSXCalendarTimeUTC, SMSXAdvertising);
  }

  public SmsXSenderResponse sendPaidSms(String oa, String da, String message, boolean SMSXExpress, boolean SMSXSecret, boolean SMSXCalendar, long SMSXCalendarTimeUTC) throws RemoteException {
    return impl.sendPaidSms(oa, da, message, SMSXExpress, SMSXSecret, SMSXCalendar, SMSXCalendarTimeUTC);
  }

  public SmsXSenderResponse checkStatus(java.lang.String SMSXIdMessage) throws java.rmi.RemoteException {
    return impl.checkStatus(SMSXIdMessage);
  }

  public SmsXSenderResponse batchSecret(String oa, String message, boolean express) throws RemoteException {
    MessageContext ctx = MessageContext.getCurrentContext();
    Message reqMessage = ctx.getRequestMessage();
    Attachments attachments = reqMessage.getAttachmentsImpl();
    AttachmentPart attachement = (AttachmentPart)attachments.getAttachmentByReference("destinations");

    InputStream is = null;
    if (attachement != null) {
      try {
        is = attachement.getDataHandler().getInputStream();
      } catch (Exception e) {
        e.printStackTrace();
        is = null;
      }
    }

    return impl.batchSecret(oa, message, express, is);  
  }

  public SmsXSenderResponse checkBatchSecretStatus(String msgId) throws RemoteException {
    return impl.checkBatchSecretStatus(msgId);
  }

  public SmsXSenderResponse sendSysSms(String oa, String da, String message) throws RemoteException {
    return impl.sendSysSms(oa, da, message);
  }

}
