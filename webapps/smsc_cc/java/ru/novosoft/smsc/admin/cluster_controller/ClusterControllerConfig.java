package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.config.XmlConfig;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * @author Artem Snopkov
 */
class ClusterControllerConfig implements ManagedConfigFile<ClusterControllerSettings> {

  public void save(InputStream oldFile, OutputStream newFile, ClusterControllerSettings settings) throws Exception {
    XmlConfig config = new XmlConfig(oldFile);
    config.getSection("listener").setInt("port", settings.getListenerPort());
    config.getSection("listener").setString("host", settings.getListenerHost());
    config.save(newFile);
  }

  public ClusterControllerSettings load(InputStream is) throws Exception {
    ClusterControllerSettings s = new ClusterControllerSettings();
    XmlConfig config = new XmlConfig(is);
    s.setListenerPort(config.getSection("listener").getInt("port"));
    s.setListenerHost(config.getSection("listener").getString("host"));
    return s;
  }
}
