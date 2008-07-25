package ru.sibinco.smsx.engine.soaphandler.groupsend;

public interface GroupSend extends java.rmi.Remote {
  public GroupSendResp sendSms(String groupName, String owner, String message, boolean express) throws java.rmi.RemoteException;
  public GroupSendResp checkStatus(String msgId) throws java.rmi.RemoteException;
}
