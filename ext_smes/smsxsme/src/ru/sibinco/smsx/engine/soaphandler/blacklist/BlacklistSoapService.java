/**
 * BlacklistSoapService.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.blacklist;

import java.rmi.RemoteException;

public class BlacklistSoapService implements BlacklistSoap, org.apache.axis.wsdl.Skeleton {
  private BlacklistSoap impl;
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
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "msisdn"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("add", _params, new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "AddResult"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "Add"));
//    _oper.setSoapAction("http://mts.qmobile.ru/api/Add");
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("add") == null) {
      _myOperations.put("add", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("add")).add(_oper);
    _params = new org.apache.axis.description.ParameterDesc [] {
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "msisdn"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("remove", _params, new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "RemoveResult"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "Remove"));
//    _oper.setSoapAction("http://mts.qmobile.ru/api/Remove");
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("remove") == null) {
      _myOperations.put("remove", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("remove")).add(_oper);
    _params = new org.apache.axis.description.ParameterDesc [] {
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "msisdn"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("check", _params, new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "CheckResult"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://mts.qmobile.ru/api/", "Check"));
//    _oper.setSoapAction("http://mts.qmobile.ru/api/Remove");
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("check") == null) {
      _myOperations.put("check", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("check")).add(_oper);
  }

  public BlacklistSoapService() {
    this.impl = BlacklistSoapFactory.createBlacklistSoapHandler();
  }

  public boolean add(String msisdn) throws java.rmi.RemoteException {
    return impl.add(msisdn);
  }

  public boolean remove(String msisdn) throws java.rmi.RemoteException {
    return impl.remove(msisdn);
  }

  public boolean check(String msisdn) throws RemoteException {
    return impl.check(msisdn);
  }
}
