/**
 * BlacklistLocator.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.blacklist;

import java.net.URL;

public class BlacklistLocator extends org.apache.axis.client.Service implements Blacklist {

    public BlacklistLocator() {
    }


    public BlacklistLocator(org.apache.axis.EngineConfiguration config) {
        super(config);
    }

    public BlacklistLocator(java.lang.String wsdlLoc, javax.xml.namespace.QName sName) throws javax.xml.rpc.ServiceException {
        super(wsdlLoc, sName);
    }

    // Use to get a proxy class for blacklistSoap
    private java.lang.String blacklistSoap_address = "http://dev.mts.qmobile.ru/api/blacklist.asmx";

    public java.lang.String getblacklistSoapAddress() {
        return blacklistSoap_address;
    }

    // The WSDD service name defaults to the port name.
    private java.lang.String blacklistSoapWSDDServiceName = "blacklistSoap";

    public java.lang.String getblacklistSoapWSDDServiceName() {
        return blacklistSoapWSDDServiceName;
    }

    public void setblacklistSoapWSDDServiceName(java.lang.String name) {
        blacklistSoapWSDDServiceName = name;
    }

    public BlacklistSoap getblacklistSoap() throws javax.xml.rpc.ServiceException {
       java.net.URL endpoint;
        try {
            endpoint = new java.net.URL(blacklistSoap_address);
        }
        catch (java.net.MalformedURLException e) {
            throw new javax.xml.rpc.ServiceException(e);
        }
        return getblacklistSoap(endpoint);
    }

    public BlacklistSoap getblacklistSoap(URL portAddress) throws javax.xml.rpc.ServiceException {
        try {
            BlacklistSoapStub _stub = new BlacklistSoapStub(portAddress, this);
            _stub.setPortName(getblacklistSoapWSDDServiceName());
            return _stub;
        }
        catch (org.apache.axis.AxisFault e) {
            return null;
        }
    }

    public void setblacklistSoapEndpointAddress(java.lang.String address) {
        blacklistSoap_address = address;
    }


    // Use to get a proxy class for blacklistSoap12
    private String blacklistSoap12_address = "http://dev.mts.qmobile.ru/api/blacklist.asmx";

    public java.lang.String getblacklistSoap12Address() {
        return blacklistSoap12_address;
    }

    // The WSDD service name defaults to the port name.
    private java.lang.String blacklistSoap12WSDDServiceName = "blacklistSoap12";

    public java.lang.String getblacklistSoap12WSDDServiceName() {
        return blacklistSoap12WSDDServiceName;
    }

    public void setblacklistSoap12WSDDServiceName(java.lang.String name) {
        blacklistSoap12WSDDServiceName = name;
    }

    public BlacklistSoap getblacklistSoap12() throws javax.xml.rpc.ServiceException {
       java.net.URL endpoint;
        try {
            endpoint = new java.net.URL(blacklistSoap12_address);
        }
        catch (java.net.MalformedURLException e) {
            throw new javax.xml.rpc.ServiceException(e);
        }
        return getblacklistSoap12(endpoint);
    }

    public BlacklistSoap getblacklistSoap12(java.net.URL portAddress) throws javax.xml.rpc.ServiceException {
        try {
            BlacklistSoap12Stub _stub = new BlacklistSoap12Stub(portAddress, this);
            _stub.setPortName(getblacklistSoap12WSDDServiceName());
            return _stub;
        }
        catch (org.apache.axis.AxisFault e) {
            return null;
        }
    }

    public void setblacklistSoap12EndpointAddress(java.lang.String address) {
        blacklistSoap12_address = address;
    }

    /**
     * For the given interface, get the stub implementation.
     * If this service has no port for the given interface,
     * then ServiceException is thrown.
     * This service has multiple ports for a given interface;
     * the proxy implementation returned may be indeterminate.
     */
    public java.rmi.Remote getPort(Class serviceEndpointInterface) throws javax.xml.rpc.ServiceException {
        try {
            if (BlacklistSoap.class.isAssignableFrom(serviceEndpointInterface)) {
                BlacklistSoapStub _stub = new BlacklistSoapStub(new java.net.URL(blacklistSoap_address), this);
                _stub.setPortName(getblacklistSoapWSDDServiceName());
                return _stub;
            }
            if (BlacklistSoap.class.isAssignableFrom(serviceEndpointInterface)) {
                BlacklistSoap12Stub _stub = new BlacklistSoap12Stub(new java.net.URL(blacklistSoap12_address), this);
                _stub.setPortName(getblacklistSoap12WSDDServiceName());
                return _stub;
            }
        }
        catch (java.lang.Throwable t) {
            throw new javax.xml.rpc.ServiceException(t);
        }
        throw new javax.xml.rpc.ServiceException("There is no stub implementation for the interface:  " + (serviceEndpointInterface == null ? "null" : serviceEndpointInterface.getName()));
    }

    /**
     * For the given interface, get the stub implementation.
     * If this service has no port for the given interface,
     * then ServiceException is thrown.
     */
    public java.rmi.Remote getPort(javax.xml.namespace.QName portName, Class serviceEndpointInterface) throws javax.xml.rpc.ServiceException {
        if (portName == null) {
            return getPort(serviceEndpointInterface);
        }
        java.lang.String inputPortName = portName.getLocalPart();
        if ("blacklistSoap".equals(inputPortName)) {
            return getblacklistSoap();
        }
        else if ("blacklistSoap12".equals(inputPortName)) {
            return getblacklistSoap12();
        }
        else  {
            java.rmi.Remote _stub = getPort(serviceEndpointInterface);
            ((org.apache.axis.client.Stub) _stub).setPortName(portName);
            return _stub;
        }
    }

    public javax.xml.namespace.QName getServiceName() {
        return new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "blacklist");
    }

    private java.util.HashSet ports = null;

    public java.util.Iterator getPorts() {
        if (ports == null) {
            ports = new java.util.HashSet();
            ports.add(new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "blacklistSoap"));
            ports.add(new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "blacklistSoap12"));
        }
        return ports.iterator();
    }

    /**
    * Set the endpoint address for the specified port name.
    */
    public void setEndpointAddress(java.lang.String portName, java.lang.String address) throws javax.xml.rpc.ServiceException {
        
if ("blacklistSoap".equals(portName)) {
            setblacklistSoapEndpointAddress(address);
        }
        else 
if ("blacklistSoap12".equals(portName)) {
            setblacklistSoap12EndpointAddress(address);
        }
        else 
{ // Unknown Port Name
            throw new javax.xml.rpc.ServiceException(" Cannot set Endpoint Address for Unknown Port" + portName);
        }
    }

    /**
    * Set the endpoint address for the specified port name.
    */
    public void setEndpointAddress(javax.xml.namespace.QName portName, java.lang.String address) throws javax.xml.rpc.ServiceException {
        setEndpointAddress(portName.getLocalPart(), address);
    }

}
