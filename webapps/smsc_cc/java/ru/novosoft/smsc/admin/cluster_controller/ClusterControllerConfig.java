package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.config.XmlConfig;

import java.io.InputStream;
import java.io.OutputStream;

/**
 * @author Artem Snopkov
 */
class ClusterControllerConfig implements ManagedConfigFile {

  private ClusterControllerSettings settings;

  public void save(InputStream oldFile, OutputStream newFile) throws Exception {
    XmlConfig config = new XmlConfig(oldFile);
    config.getSection("listener").setInt("port", settings.getListenerPort());
    config.save(newFile);
  }

  public void load(InputStream is) throws Exception {
    ClusterControllerSettings s = new ClusterControllerSettings();
    XmlConfig config = new XmlConfig(is);
    s.setListenerPort(config.getSection("listener").getInt("port"));
    settings = s;
  }

  public ClusterControllerSettings getSettings() {
    return settings;
  }

  public void setSettings(ClusterControllerSettings settings) {
    this.settings = settings;
  }
}
