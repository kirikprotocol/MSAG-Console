package ru.sibinco.smsx.engine.soaphandler.groupedit;

import java.net.URL;

public interface GroupEditService extends javax.xml.rpc.Service {
  public String getGroupEditAddress();

  public GroupEdit getGroupEdit() throws javax.xml.rpc.ServiceException;

  public GroupEdit getGroupEdit(URL portAddress) throws javax.xml.rpc.ServiceException;
}
