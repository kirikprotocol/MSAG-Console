/**
 * SmsXSenderService.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.smsxsender;

public interface SmsXSenderService extends javax.xml.rpc.Service {
    public java.lang.String getSmsXSenderAddress();

    public SmsXSender getSmsXSender() throws javax.xml.rpc.ServiceException;

    public SmsXSender getSmsXSender(java.net.URL portAddress) throws javax.xml.rpc.ServiceException;
}
