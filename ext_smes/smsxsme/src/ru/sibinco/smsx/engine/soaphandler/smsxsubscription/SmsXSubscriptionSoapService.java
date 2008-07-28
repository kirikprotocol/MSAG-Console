package ru.sibinco.smsx.engine.soaphandler.smsxsubscription;

public class SmsXSubscriptionSoapService implements SmsXSubscription, org.apache.axis.wsdl.Skeleton {
  private SmsXSubscription impl;
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
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/smsxsubscription", "msisdn"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("checkSubscription", _params, new javax.xml.namespace.QName("http://sibinco.ru/smsxsubscription", "checkSubscriptionReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/smsxsubscription", "CheckSubscriptionResp"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/smsxsubscription", "checkSubscription"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("checkSubscription") == null) {
      _myOperations.put("checkSubscription", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("checkSubscription")).add(_oper);
  }

  public SmsXSubscriptionSoapService() {
    this.impl = new SmsXSubscriptionSoapHandler();
  }

  public SmsXSubscriptionSoapService(SmsXSubscription impl) {
    this.impl = impl;
  }
  public CheckSubscriptionResp checkSubscription(java.lang.String msisdn) throws java.rmi.RemoteException
  {
    CheckSubscriptionResp ret = impl.checkSubscription(msisdn);
    return ret;
  }

}
