package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import ru.novosoft.smsc.util.config.XmlConfigParam;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;
import java.util.TreeMap;

/**
 * Класс для работы с конфигурационным файлом webconfig.xml
 * @author Artem Snopkov
 */
class AdminContextConfig {

  private final XmlConfig webconfig;

  public AdminContextConfig(File file) throws AdminException {
    this.webconfig = new XmlConfig();

    try {
      this.webconfig.load(file);
    } catch (XmlConfigException e) {
      throw new AdminContextException("Unable to load " + file.getAbsolutePath() + ".Cause: " + e.getMessage(), e);
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

  private static final String perfMonPref = "appletPort";

  public int[] getPerfMonitorPorts() throws AdminException {
    try {
      XmlConfigSection perfmon = webconfig.getSection("perfmon");
      Map<Integer, Integer> m = new TreeMap<Integer, Integer>();
      for(XmlConfigParam p : perfmon.params()){
        if(p.getName().startsWith(monAppletPortPref)) {
          m.put(Integer.parseInt(p.getName().substring(monAppletPortPref.length())), p.getInt());
        }
      }
      int[] res = new int[m.size()];
      int i = 0;
      for(int p : m.values()) {
        res[i] = p;
        i++;
      }
      return res;
    } catch (NumberFormatException e){
      throw new AdminContextException("invalid_config", e);
    } catch (XmlConfigException e) {
      throw new AdminContextException("invalid_config", e);
    }
  }

  private static final String monAppletPortPref = "appletPort";

  public int[] getTopMonitorPorts() throws AdminException {
    try {
      XmlConfigSection perfmon = webconfig.getSection("topmon");
      Map<Integer, Integer> m = new TreeMap<Integer, Integer>();
      for(XmlConfigParam p : perfmon.params()){
        if(p.getName().startsWith(monAppletPortPref)) {
          m.put(Integer.parseInt(p.getName().substring(monAppletPortPref.length())), p.getInt());
        }
      }
      int[] res = new int[m.size()];
      int i = 0;
      for(int p : m.values()) {
        res[i] = p;
        i++;
      }
      return res;
    } catch (NumberFormatException e){
      throw new AdminContextException("invalid_config", e);
    } catch (XmlConfigException e) {
      throw new AdminContextException("invalid_config", e);
    }
  }

  public boolean isPerfMonSupport64Bit() throws AdminException {
    try {
      XmlConfigSection perfmon = webconfig.getSection("perfmon");
      return perfmon.getBool("support64Bit");
    } catch (XmlConfigException e) {
      throw new AdminContextException("invalid_config", e);
    }
  }

}
