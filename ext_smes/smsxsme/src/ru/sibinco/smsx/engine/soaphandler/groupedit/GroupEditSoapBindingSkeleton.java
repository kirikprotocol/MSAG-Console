package ru.sibinco.smsx.engine.soaphandler.groupedit;

public class GroupEditSoapBindingSkeleton implements GroupEdit, org.apache.axis.wsdl.Skeleton {
  private GroupEdit impl;
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
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "owner"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("groupList", _params, new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groupListReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "GroupListResp"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groupList"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("groupList") == null) {
      _myOperations.put("groupList", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("groupList")).add(_oper);
    _params = new org.apache.axis.description.ParameterDesc [] {
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groupName"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "owner"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("addGroup", _params, new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "addGroupReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "int"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "addGroup"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("addGroup") == null) {
      _myOperations.put("addGroup", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("addGroup")).add(_oper);
    _params = new org.apache.axis.description.ParameterDesc [] {
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groupName"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "owner"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("removeGroup", _params, new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "removeGroupReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "int"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "removeGroup"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("removeGroup") == null) {
      _myOperations.put("removeGroup", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("removeGroup")).add(_oper);
    _params = new org.apache.axis.description.ParameterDesc [] {
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groupName"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "owner"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("groupInfo", _params, new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groupInfoReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "GroupInfoResp"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groupInfo"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("groupInfo") == null) {
      _myOperations.put("groupInfo", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("groupInfo")).add(_oper);
    _params = new org.apache.axis.description.ParameterDesc [] {
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groupName"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "owner"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "member"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("addMember", _params, new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "addMemberReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "int"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "addMember"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("addMember") == null) {
      _myOperations.put("addMember", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("addMember")).add(_oper);
    _params = new org.apache.axis.description.ParameterDesc [] {
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groupName"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "owner"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "member"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("removeMember", _params, new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "removeMemberReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "int"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "removeMember"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("removeMember") == null) {
      _myOperations.put("removeMember", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("removeMember")).add(_oper);
    _params = new org.apache.axis.description.ParameterDesc [] {
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "groupName"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "owner"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
        new org.apache.axis.description.ParameterDesc(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "member"), org.apache.axis.description.ParameterDesc.IN, new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "string"), java.lang.String.class, false, false),
    };
    _oper = new org.apache.axis.description.OperationDesc("renameGroup", _params, new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "renameGroupReturn"));
    _oper.setReturnType(new javax.xml.namespace.QName("http://www.w3.org/2001/XMLSchema", "int"));
    _oper.setElementQName(new javax.xml.namespace.QName("http://sibinco.ru/groupedit", "renameGroup"));
    _oper.setSoapAction("");
    _myOperationsList.add(_oper);
    if (_myOperations.get("renameGroup") == null) {
      _myOperations.put("renameGroup", new java.util.ArrayList());
    }
    ((java.util.List)_myOperations.get("renameGroup")).add(_oper);
  }

  public GroupEditSoapBindingSkeleton() {
    this.impl = GroupEditFactory.createGroupEdit();
  }

  public GroupEditSoapBindingSkeleton(GroupEdit impl) {
    this.impl = impl;
  }
  public GroupListResp groupList(java.lang.String owner) throws java.rmi.RemoteException
  {
    GroupListResp ret = impl.groupList(owner);
    return ret;
  }

  public int addGroup(String groupName, String owner) throws java.rmi.RemoteException
  {
    int ret = impl.addGroup(groupName, owner);
    return ret;
  }

  public int removeGroup(java.lang.String groupName, java.lang.String owner) throws java.rmi.RemoteException
  {
    int ret = impl.removeGroup(groupName, owner);
    return ret;
  }

  public GroupInfoResp groupInfo(java.lang.String groupName, java.lang.String owner) throws java.rmi.RemoteException
  {
    GroupInfoResp ret = impl.groupInfo(groupName, owner);
    return ret;
  }

  public int addMember(java.lang.String groupName, java.lang.String owner, java.lang.String member) throws java.rmi.RemoteException
  {
    int ret = impl.addMember(groupName, owner, member);
    return ret;
  }

  public int removeMember(java.lang.String groupName, java.lang.String owner, java.lang.String member) throws java.rmi.RemoteException
  {
    int ret = impl.removeMember(groupName, owner, member);
    return ret;
  }

  public int renameGroup(java.lang.String groupName, java.lang.String owner, java.lang.String member) throws java.rmi.RemoteException
  {
    int ret = impl.renameGroup(groupName, owner, member);
    return ret;
  }

}
