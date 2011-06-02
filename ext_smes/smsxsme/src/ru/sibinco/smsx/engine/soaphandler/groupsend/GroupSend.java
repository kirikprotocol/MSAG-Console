/**
 * GroupSend_PortType.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.groupsend;

public interface GroupSend extends java.rmi.Remote {
    public ru.sibinco.smsx.engine.soaphandler.groupsend.GroupSendResp sendSms(java.lang.String groupName, java.lang.String owner, java.lang.String message, boolean express) throws java.rmi.RemoteException;
    public ru.sibinco.smsx.engine.soaphandler.groupsend.GroupSendResp checkStatus(java.lang.String msgId) throws java.rmi.RemoteException;
}
