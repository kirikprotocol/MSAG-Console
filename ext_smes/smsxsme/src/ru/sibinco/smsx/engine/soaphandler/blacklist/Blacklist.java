/**
 * Blacklist.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.blacklist;

public interface Blacklist extends javax.xml.rpc.Service {
    public java.lang.String getblacklistSoapAddress();

    public BlacklistSoap getblacklistSoap() throws javax.xml.rpc.ServiceException;

    public BlacklistSoap getblacklistSoap(java.net.URL portAddress) throws javax.xml.rpc.ServiceException;
    public String getblacklistSoap12Address();

    public BlacklistSoap getblacklistSoap12() throws javax.xml.rpc.ServiceException;

    public BlacklistSoap getblacklistSoap12(java.net.URL portAddress) throws javax.xml.rpc.ServiceException;
}
