package ru.sibinco.smsx.engine.soaphandler.smsxsubscription;

public class SmsXSubscriptionServiceLocator extends org.apache.axis.client.Service implements SmsXSubscriptionService {

  public SmsXSubscriptionServiceLocator() {
  }


  public SmsXSubscriptionServiceLocator(org.apache.axis.EngineConfiguration config) {
    super(config);
  }

  public SmsXSubscriptionServiceLocator(java.lang.String wsdlLoc, javax.xml.namespace.QName sName) throws javax.xml.rpc.ServiceException {
    super(wsdlLoc, sName);
  }

  // Use to get a proxy class for SmsXSubscription
  private java.lang.String SmsXSubscription_address = "http://localhost:8080/SmsXSubscription";

  public java.lang.String getSmsXSubscriptionAddress() {
    return SmsXSubscription_address;
  }

  // The WSDD service name defaults to the port name.
  private java.lang.String SmsXSubscriptionWSDDServiceName = "SmsXSubscription";

  public java.lang.String getSmsXSubscriptionWSDDServiceName() {
    return SmsXSubscriptionWSDDServiceName;
  }

  public void setSmsXSubscriptionWSDDServiceName(java.lang.String name) {
    SmsXSubscriptionWSDDServiceName = name;
  }

  public SmsXSubscription getSmsXSubscription() throws javax.xml.rpc.ServiceException {
    java.net.URL endpoint;
    try {
      endpoint = new java.net.URL(SmsXSubscription_address);
    }
    catch (java.net.MalformedURLException e) {
      throw new javax.xml.rpc.ServiceException(e);
    }
    return getSmsXSubscription(endpoint);
  }

  public SmsXSubscription getSmsXSubscription(java.net.URL portAddress) throws javax.xml.rpc.ServiceException {
    try {
      SmsXSubscriptionSoapBindingStub _stub = new SmsXSubscriptionSoapBindingStub(portAddress, this);
      _stub.setPortName(getSmsXSubscriptionWSDDServiceName());
      return _stub;
    }
    catch (org.apache.axis.AxisFault e) {
      return null;
    }
  }

  public void setSmsXSubscriptionEndpointAddress(java.lang.String address) {
    SmsXSubscription_address = address;
  }

  /**
   * For the given interface, get the stub implementation.
   * If this service has no port for the given interface,
   * then ServiceException is thrown.
   */
  public java.rmi.Remote getPort(Class serviceEndpointInterface) throws javax.xml.rpc.ServiceException {
    try {
      if (SmsXSubscription.class.isAssignableFrom(serviceEndpointInterface)) {
        SmsXSubscriptionSoapBindingStub _stub = new SmsXSubscriptionSoapBindingStub(new java.net.URL(SmsXSubscription_address), this);
        _stub.setPortName(getSmsXSubscriptionWSDDServiceName());
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
    if ("SmsXSubscription".equals(inputPortName)) {
      return getSmsXSubscription();
    }
    else  {
      java.rmi.Remote _stub = getPort(serviceEndpointInterface);
      ((org.apache.axis.client.Stub) _stub).setPortName(portName);
      return _stub;
    }
  }

  public javax.xml.namespace.QName getServiceName() {
    return new javax.xml.namespace.QName("http://sibinco.ru/smsxsubscription", "SmsXSubscriptionService");
  }

  private java.util.HashSet ports = null;

  public java.util.Iterator getPorts() {
    if (ports == null) {
      ports = new java.util.HashSet();
      ports.add(new javax.xml.namespace.QName("http://sibinco.ru/smsxsubscription", "SmsXSubscription"));
    }
    return ports.iterator();
  }

  /**
   * Set the endpoint address for the specified port name.
   */
  public void setEndpointAddress(java.lang.String portName, java.lang.String address) throws javax.xml.rpc.ServiceException {

    if ("SmsXSubscription".equals(portName)) {
      setSmsXSubscriptionEndpointAddress(address);
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
