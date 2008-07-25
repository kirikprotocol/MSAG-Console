package ru.sibinco.smsx.engine.soaphandler.groupsend;

public class GroupSendSoapService implements GroupSend, org.apache.axis.wsdl.Skeleton {
  private GroupSend impl;
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
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "groupName"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "owner"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "message"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "express"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "boolean"), boolean.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("sendSms", _params, new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "sendSmsReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "GroupSendResp"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "sendSms"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("sendSms") == null) {
      _myOperations.put("sendSms", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("sendSms")).add(_oper);
    _params = new org.apache.axis.description.ParameterDesc [] {
      new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "msgId"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("checkStatus", _params, new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "checkStatusReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "GroupSendResp"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/groupsend", "checkStatus"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("checkStatus") == null) {
      _myOperations.put("checkStatus", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("checkStatus")).add(_oper);
  }

  public GroupSendSoapService() {
    this.impl = GroupSendFactory.createGroupSend();
  }

  public GroupSendSoapService(GroupSend impl) {
    this.impl = impl;
  }

  public GroupSendResp sendSms(String groupName, String owner, String message, boolean express) throws java.rmi.RemoteException {
    return impl.sendSms(groupName, owner, message, express);
  }

  public GroupSendResp checkStatus(String msgId) throws java.rmi.RemoteException {
    return impl.checkStatus(msgId);
  }
}
