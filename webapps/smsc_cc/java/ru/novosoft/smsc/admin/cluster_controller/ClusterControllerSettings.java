package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

/**
 * @author Artem Snopkov
 */
public class ClusterControllerSettings {

  private static final ValidationHelper vh = new ValidationHelper(ClusterControllerSettings.class);

  private int listenerPort;
  private String listenerHost;

  ClusterControllerSettings() {
  }

  ClusterControllerSettings(ClusterControllerSettings c) {
    this.listenerPort = c.listenerPort;
    this.listenerHost = c.listenerHost;
  }

  public int getListenerPort() {
    return listenerPort;
  }

  public void setListenerPort(int listenerPort) throws AdminException {
    vh.checkPort("listenerPort", listenerPort);
    this.listenerPort = listenerPort;
  }

  public String getListenerHost() {
    return listenerHost;
  }

  public void setListenerHost(String listenerHost) throws AdminException {
    vh.checkNotEmpty("listenerHost", listenerHost);
    this.listenerHost = listenerHost;
  }
}
