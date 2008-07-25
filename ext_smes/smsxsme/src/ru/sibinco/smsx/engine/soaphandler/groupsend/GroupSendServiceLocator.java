package ru.sibinco.smsx.engine.soaphandler.groupsend;

public class GroupSendServiceLocator extends org.apache.axis.client.Service implements GroupSendService {

  public GroupSendServiceLocator() {
  }


  public GroupSendServiceLocator(org.apache.axis.EngineConfiguration config) {
    super(config);
  }

  public GroupSendServiceLocator(java.lang.String wsdlLoc, javax.xml.namespace.QName sName) throws javax.xml.rpc.ServiceException {
    super(wsdlLoc, sName);
  }

  // Use to get a proxy class for GroupSend
  private java.lang.String GroupSend_address = "http://localhost:8080/GroupSend";

  public java.lang.String getGroupSendAddress() {
    return GroupSend_address;
  }

  // The WSDD service name defaults to the port name.
  private java.lang.String GroupSendWSDDServiceName = "GroupSend";

  public java.lang.String getGroupSendWSDDServiceName() {
    return GroupSendWSDDServiceName;
  }

  public void setGroupSendWSDDServiceName(java.lang.String name) {
    GroupSendWSDDServiceName = name;
  }

  public GroupSend getGroupSend() throws javax.xml.rpc.ServiceException {
    java.net.URL endpoint;
    try {
      endpoint = new java.net.URL(GroupSend_address);
    }
    catch (java.net.MalformedURLException e) {
      throw new javax.xml.rpc.ServiceException(e);
    }
    return getGroupSend(endpoint);
  }

  public GroupSend getGroupSend(java.net.URL portAddress) throws javax.xml.rpc.ServiceException {
    try {
      GroupSendSoapBindingStub _stub = new GroupSendSoapBindingStub(portAddress, this);
      _stub.setPortName(getGroupSendWSDDServiceName());
      return _stub;
    }
    catch (org.apache.axis.AxisFault e) {
      return null;
    }
  }

  public void setGroupSendEndpointAddress(java.lang.String address) {
    GroupSend_address = address;
  }

  /**
   * For the given interface, get the stub implementation.
   * If this service has no port for the given interface,
   * then ServiceException is thrown.
   */
  public java.rmi.Remote getPort(Class serviceEndpointInterface) throws javax.xml.rpc.ServiceException {
    try {
      if (GroupSend.class.isAssignableFrom(serviceEndpointInterface)) {
        GroupSendSoapBindingStub _stub = new GroupSendSoapBindingStub(new java.net.URL(GroupSend_address), this);
        _stub.setPortName(getGroupSendWSDDServiceName());
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
    if ("GroupSend".equals(inputPortName)) {
      return getGroupSend();
    }
    else  {
      java.rmi.Remote _stub = getPort(serviceEndpointInterface);
      ((org.apache.axis.client.Stub) _stub).setPortName(portName);
      return _stub;
    }
  }

  public javax.xml.namespace.QName getServiceName() {
    return new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "GroupSendService");
  }

  private java.util.HashSet ports = null;

  public java.util.Iterator getPorts() {
    if (ports == null) {
      ports = new java.util.HashSet();
      ports.add(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "GroupSend"));
    }
    return ports.iterator();
  }

  /**
   * Set the endpoint address for the specified port name.
   */
  public void setEndpointAddress(java.lang.String portName, java.lang.String address) throws javax.xml.rpc.ServiceException {

    if ("GroupSend".equals(portName)) {
      setGroupSendEndpointAddress(address);
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
