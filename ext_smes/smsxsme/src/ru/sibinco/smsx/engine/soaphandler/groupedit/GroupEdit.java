package ru.sibinco.smsx.engine.soaphandler.groupedit;

public interface GroupEdit extends java.rmi.Remote {
  public GroupListResp groupList(String owner) throws java.rmi.RemoteException;
  public int addGroup(String groupName, String owner) throws java.rmi.RemoteException;
  public int removeGroup(String groupName, String owner) throws java.rmi.RemoteException;
  public GroupInfoResp groupInfo(String groupName, String owner) throws java.rmi.RemoteException;
  public int addMember(String groupName, String owner, String member) throws java.rmi.RemoteException;
  public int removeMember(String groupName, String owner, String member) throws java.rmi.RemoteException;
  public int renameGroup(String groupName, String owner, String newName) throws java.rmi.RemoteException;
  public int copyGroup(String groupName, String owner, String newName) throws java.rmi.RemoteException;
}
