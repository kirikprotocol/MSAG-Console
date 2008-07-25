package ru.sibinco.smsx.engine.soaphandler.groupsend;

public interface GroupSendService extends javax.xml.rpc.Service {
  public String getGroupSendAddress();

  public GroupSend getGroupSend() throws javax.xml.rpc.ServiceException;

  public GroupSend getGroupSend(java.net.URL portAddress) throws javax.xml.rpc.ServiceException;
}
