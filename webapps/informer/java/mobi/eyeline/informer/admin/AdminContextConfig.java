package mobi.eyeline.informer.admin;

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
 * Класс для работы с конфигурационным файлом webconfig.xml
 * @author Artem Snopkov
 */
class AdminContextConfig {

  private final XmlConfig webconfig = new XmlConfig();

  public AdminContextConfig(InputStream is) throws AdminException{
    load(is);
  }

  public AdminContextConfig(File file) throws AdminException {
    if(file == null) {
      throw new IllegalArgumentException("Some arguments are null");      
    }
    InputStream is = null;
    try{
      is = new FileInputStream(file);
      load(is);
    }catch (IOException e){
      throw new AdminContextException("Unable to load " + file.getAbsolutePath() + ".Cause: " + e.getMessage(), e);
    }catch (AdminException e){
      throw new AdminContextException("Unable to load " + file.getAbsolutePath() + ".Cause: " + e.getMessage(), e);
    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException e){}
      }
    }
  }

  private void load(InputStream is) throws AdminException{
    if(is == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    try {
      this.webconfig.load(is);
    } catch (XmlConfigException e) {
      throw new AdminContextException("Unable to load .Cause: " + e.getMessage(), e);
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
        throw new AdminContextException("unknown_inst_type", installationTypeStr);
    } catch (XmlConfigException e) {
      throw new AdminContextException("invalid_config", e);
    }
  }


  public String getSingleDaemonHost() throws AdminException {
    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getString("host");
    } catch (XmlConfigException e) {
      throw new AdminContextException("invalid_config", e);
    }
  }


  public int getSingleDaemonPort() throws AdminException {
    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getInt("port");
    } catch (XmlConfigException e) {
      throw new AdminContextException("invalid_config", e);
    }
  }


  public File[] getAppMirrorDirs() throws AdminException {
    try {
      XmlConfigSection installation = webconfig.getSection("installation");
      File mirrorPath = new File(installation.getString("mirrorpath"));
      return new File[]{mirrorPath};
    } catch (XmlConfigException e) {
      throw new AdminContextException("invalid_config", e);
    }
  }


  public String getHSDaemonHost() throws AdminException {
    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getString("host");
    } catch (XmlConfigException e) {
      throw new AdminContextException("invalid_config", e);
    }
  }

  public int getHSDaemonPort() throws AdminException {
    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getInt("port");
    } catch (XmlConfigException e) {
      throw new AdminContextException("invalid_config", e);
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
      throw new AdminContextException("invalid_config", e);
    }
  }

  public String getUsersFile() throws AdminException {
    try {
      XmlConfigSection system = webconfig.getSection("system");
      return system.getString("users file");
    } catch (XmlConfigException e) {
      throw new AdminContextException("invalid_config", e);
    }    
  }


}
