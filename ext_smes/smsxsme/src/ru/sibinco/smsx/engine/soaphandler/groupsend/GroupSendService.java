/**
 * GroupSendService.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.groupsend;

public interface GroupSendService extends javax.xml.rpc.Service {
    public java.lang.String getGroupSendAddress();

    public GroupSend getGroupSend() throws javax.xml.rpc.ServiceException;

    public GroupSend getGroupSend(java.net.URL portAddress) throws javax.xml.rpc.ServiceException;
}
