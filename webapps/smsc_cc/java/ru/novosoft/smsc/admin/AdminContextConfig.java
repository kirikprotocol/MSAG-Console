package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.util.config.XmlConfigException;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
class AdminContextConfig {

  private final XmlConfig webconfig;

  public AdminContextConfig(File file) throws AdminException {
    this.webconfig = new XmlConfig();

    try {
      this.webconfig.load(file);
    } catch (XmlConfigException e) {
      throw new AdminException("Unable to load " + file.getAbsolutePath() + ".Cause: " + e.getMessage(), e);
    }
  }

  public InstallationType getInstallationType() throws AdminException {
    try {
      String installationTypeStr = webconfig.getSection("installation").getString("type", "single");
      if (installationTypeStr.equalsIgnoreCase("hs"))
        return InstallationType.HS;
      else if (installationTypeStr.equalsIgnoreCase("ha"))
        return InstallationType.HA;
      else if (installationTypeStr.equalsIgnoreCase("single"))
        return InstallationType.SINGLE;
      else
        throw new AdminException("Unknown installation type: " + installationTypeStr);
    } catch (XmlConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }


  public String getSingleDaemonHost() throws AdminException {
    if (getInstallationType() != InstallationType.SINGLE)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getString("host");
    } catch (XmlConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }


  public int getSingleDaemonPort() throws AdminException {
    if (getInstallationType() != InstallationType.SINGLE)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getInt("port");
    } catch (XmlConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }


  public File[] getAppMirrorDirs() throws AdminException {
    if (getInstallationType() != InstallationType.HS)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection installation = webconfig.getSection("installation");
      File mirrorPath = new File(installation.getString("mirrorpath"));
      return new File[]{mirrorPath};
    } catch (XmlConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }


  public String getHSDaemonHost() throws AdminException {
    if (getInstallationType() != InstallationType.HS)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getString("host");
    } catch (XmlConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }

  public int getHSDaemonPort() throws AdminException {
    if (getInstallationType() != InstallationType.HS)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getInt("port");
    } catch (XmlConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }

  public Collection<String> getHSDaemonHosts() throws AdminException {
    if (getInstallationType() != InstallationType.HS)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection nodes = webconfig.getSection("nodes");
      Collection<String> result = new ArrayList<String>();
      for (XmlConfigSection nodeSection : nodes.sections())
        result.add(nodeSection.getName());

      return result;
    } catch (XmlConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }


}
