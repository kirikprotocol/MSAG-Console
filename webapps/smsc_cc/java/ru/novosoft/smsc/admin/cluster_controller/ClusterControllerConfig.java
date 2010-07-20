package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.config.XmlConfig;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * @author Artem Snopkov
 */
public class ClusterControllerConfig implements ManagedConfigFile {

  private int listenerPort;

  public void save(InputStream oldFile, OutputStream newFile) throws Exception {
    XmlConfig config = new XmlConfig(oldFile);
    config.getSection("listener").setInt("port", listenerPort);
    config.save(newFile);
  }

  public void load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig(is);
    listenerPort = config.getSection("listener").getInt("port");
  }

  public int getListenerPort() {
    return listenerPort;
  }

  public void setListenerPort(int listenerPort) {
    this.listenerPort = listenerPort;
  }
}
