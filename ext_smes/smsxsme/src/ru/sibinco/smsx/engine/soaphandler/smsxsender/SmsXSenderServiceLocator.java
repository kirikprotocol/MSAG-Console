/**
 * SmsXSenderServiceLocator.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.smsxsender;

import org.apache.axis.AxisProperties;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Date;

public class SmsXSenderServiceLocator extends org.apache.axis.client.Service implements SmsXSenderService {

  public SmsXSenderServiceLocator() {
  }


  public SmsXSenderServiceLocator(org.apache.axis.EngineConfiguration config) {
    super(config);
  }

  public SmsXSenderServiceLocator(java.lang.String wsdlLoc, javax.xml.namespace.QName sName) throws javax.xml.rpc.ServiceException {
    super(wsdlLoc, sName);
  }

  // Use to get a proxy class for SmsXSender
  private java.lang.String SmsXSender_address = "http://localhost:8080/webservice4/services/SmsXSender";

  public java.lang.String getSmsXSenderAddress() {
    return SmsXSender_address;
  }

  // The WSDD service name defaults to the port name.
  private java.lang.String SmsXSenderWSDDServiceName = "SmsXSender";

  public java.lang.String getSmsXSenderWSDDServiceName() {
    return SmsXSenderWSDDServiceName;
  }

  public void setSmsXSenderWSDDServiceName(java.lang.String name) {
    SmsXSenderWSDDServiceName = name;
  }

  public SmsXSender getSmsXSender() throws javax.xml.rpc.ServiceException {
    java.net.URL endpoint;
    try {
      endpoint = new java.net.URL(SmsXSender_address);
    }
    catch (java.net.MalformedURLException e) {
      throw new javax.xml.rpc.ServiceException(e);
    }
    return getSmsXSender(endpoint);
  }

  public SmsXSender getSmsXSender(java.net.URL portAddress) throws javax.xml.rpc.ServiceException {
    try {
      SmsXSenderStub _stub = new SmsXSenderStub(portAddress, this);
      _stub.setPortName(getSmsXSenderWSDDServiceName());
      return _stub;
    }
    catch (org.apache.axis.AxisFault e) {
      return null;
    }
  }

  public void setSmsXSenderEndpointAddress(java.lang.String address) {
    SmsXSender_address = address;
  }

  /**
   * For the given interface, get the stub implementation.
   * If this service has no port for the given interface,
   * then ServiceException is thrown.
   */
  public java.rmi.Remote getPort(Class serviceEndpointInterface) throws javax.xml.rpc.ServiceException {
    try {
      if (SmsXSender.class.isAssignableFrom(serviceEndpointInterface)) {
        SmsXSenderStub _stub = new SmsXSenderStub(new java.net.URL(SmsXSender_address), this);
        _stub.setPortName(getSmsXSenderWSDDServiceName());
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
    if ("SmsXSender".equals(inputPortName)) {
      return getSmsXSender();
    }
    else  {
      java.rmi.Remote _stub = getPort(serviceEndpointInterface);
      ((org.apache.axis.client.Stub) _stub).setPortName(portName);
      return _stub;
    }
  }

  public javax.xml.namespace.QName getServiceName() {
    return new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSenderService");
  }

  private java.util.HashSet ports = null;

  public java.util.Iterator getPorts() {
    if (ports == null) {
      ports = new java.util.HashSet();
      ports.add(new javax.xml.namespace.QName("http://sibinco.ru/smsXSend", "SmsXSender"));
    }
    return ports.iterator();
  }

  /**
   * Set the endpoint address for the specified port name.
   */
  public void setEndpointAddress(java.lang.String portName, java.lang.String address) throws javax.xml.rpc.ServiceException {

    if ("SmsXSender".equals(portName)) {
      setSmsXSenderEndpointAddress(address);
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


  public static void main(String[] args) throws Exception{
    AxisProperties.setProperty("axis.socketSecureFactory", "org.apache.axis.components.net.SunFakeTrustSocketFactory");
    SmsXSender client = new SmsXSenderServiceLocator().getSmsXSender(new URL("https://phoenix:26783/smsx/services/SmsXSender"));
    sendSms(client);
  }

  private static void sendSms(SmsXSender client) throws Exception {
    Date endDate = new SimpleDateFormat("yyyyMMddHHmm").parse("201106021601");
    PrintWriter writer = null;
    try{
      writer = new PrintWriter(new BufferedWriter(new FileWriter("test_sms_sender."+System.currentTimeMillis()+".csv")));
      String previous = null;
      do{

        try{
          SmsXSenderResponse r = client.sendSms("+79139489906", "text1", false, false, false, 0, false);
          writer.print("Create");writer.print(';');writer.print(r.getId_message());writer.print(';');writer.print(r.getSmpp_code());writer.print(';');writer.println(r.getStatus());
          if(previous != null) {
            SmsXSenderResponse resp = client.checkStatus(previous);
            writer.print("Check");writer.print(';');writer.print(previous);writer.print(';');writer.println(resp.getStatus());
            if(resp.getStatus() == 3) {
              writer.println("NOT FOUND!");
              System.out.println("NOT FOUND: id="+previous);
            }
          }
          previous = r.getId_message();

        }catch (Exception e) {
          e.printStackTrace(writer);
          System.out.println(e.getMessage());
        }
        Thread.sleep(200);
      }while (new Date().before(endDate));
    }finally {
      if(writer != null) {
        writer.close();
      }
    }
  }

}
