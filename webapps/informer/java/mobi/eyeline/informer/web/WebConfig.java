package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InstallationType;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collection;

/**
 * @author Aleksandr Khalitov
 */
@SuppressWarnings({"EmptyCatchBlock"})
public class WebConfig {


  private final XmlConfig webconfig = new XmlConfig();

  public WebConfig(InputStream is) throws InitException{
    load(is);
  }

  public WebConfig(File file) throws InitException {
    if(file == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    InputStream is = null;
    try{
      is = new FileInputStream(file);
      load(is);
    }catch (IOException e){
      throw new InitException("Unable to load " + file.getAbsolutePath() + ".Cause: " + e.getMessage(), e);
    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException e){}
      }
    }
  }

  private void load(InputStream is) throws InitException{
    if(is == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    try {
      this.webconfig.load(is);
    } catch (XmlConfigException e) {
      throw new InitException("Unable to load .Cause: " + e.getMessage(), e);
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
        throw new WebConfigException("unknown_inst_type", installationTypeStr);
    } catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }


  public String getSingleDaemonHost() throws AdminException {
    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getString("host");
    } catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }


  public int getSingleDaemonPort() throws AdminException {
    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getInt("port");
    } catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }


  public File[] getAppMirrorDirs() throws AdminException {
    try {
      XmlConfigSection installation = webconfig.getSection("installation");
      File mirrorPath = new File(installation.getString("mirrorpath"));
      return new File[]{mirrorPath};
    } catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }


  public String getHSDaemonHost() throws AdminException {
    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getString("host");
    } catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }

  public int getHSDaemonPort() throws AdminException {
    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getInt("port");
    } catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }

  public Collection<String> getHSDaemonHosts() throws AdminException {
    try {
      XmlConfigSection nodes = webconfig.getSection("nodes");
      Collection<String> result = new ArrayList<String>();
      for (XmlConfigSection nodeSection : nodes.sections())
        result.add(nodeSection.getName());

      return result;
    } catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }

  public String getUsersFile() throws AdminException {
    try {
      XmlConfigSection system = webconfig.getSection("system");
      return system.getString("users file");
    } catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }

  public String getJournalDir() throws AdminException {
    try {
      XmlConfigSection system = webconfig.getSection("system");
      return system.getString("journal dir");
    } catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }

}
