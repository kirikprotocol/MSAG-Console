package ru.novosoft.smsc.admin.smsc;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.Serializable;

/**
 * Структура с настройками для конкретного инстанца СМСЦ
 *
 * @author Artem Snopkov
 */
public class InstanceSettings implements Cloneable, Serializable {

  private String adminHost;

  private int adminPort;

  private String corePerfHost;

  private int corePerfPort;

  private String smePerfHost;

  private int smePerfPort;

  private String messageStoreStatsDir;

  private String messageStoreArchDir;

  private String localStoreFilename;

  private String localStoreMessIdFile;

  private String smppHost;

  private int smppPort;

  private String agentHost;

  private int agentPort;

  private String cacheDir;

  private String inmanHost;

  private int inmanPort;

  transient private final ValidationHelper vh = new ValidationHelper(InstanceSettings.class);

  public InstanceSettings() {
  }

  InstanceSettings(int instanceN, XmlConfig c) throws XmlConfigException {
    load(instanceN, c);
  }

  InstanceSettings(InstanceSettings s) {
    adminHost = s.adminHost;
    adminPort = s.adminPort;
    corePerfHost = s.corePerfHost;
    corePerfPort = s.corePerfPort;
    smePerfHost = s.smePerfHost;
    smePerfPort = s.smePerfPort;
    messageStoreStatsDir = s.messageStoreStatsDir;
    messageStoreArchDir = s.messageStoreArchDir;
    localStoreFilename = s.localStoreFilename;
    localStoreMessIdFile = s.localStoreMessIdFile;
    smppHost = s.smppHost;
    smppPort = s.smppPort;
    agentHost = s.agentHost;
    agentPort = s.agentPort;
    cacheDir = s.cacheDir;
  }

  protected void load(int instanceN, XmlConfig c) throws XmlConfigException {
    assert c != null : "Config is null";

    XmlConfigSection s = c.getSection("admin");
    adminHost = s.getString("host" + instanceN);
    adminPort = s.getInt("port" + instanceN);
    s = c.getSection("core").getSection("performance");
    corePerfHost = s.getString("host" + instanceN);
    corePerfPort = s.getInt("port" + instanceN);
    s = c.getSection("core").getSection("smeperformance");
    smePerfHost = s.getString("host" + instanceN);
    smePerfPort = s.getInt("port" + instanceN);
    s = c.getSection("MessageStore");
    messageStoreStatsDir = s.getString("statisticsDir" + instanceN);
    messageStoreArchDir = s.getString("archiveDir" + instanceN);
    s = c.getSection("MessageStore").getSection("LocalStore");
    localStoreFilename = s.getString("filename" + instanceN);
    localStoreMessIdFile = s.getString("msgidfile" + instanceN);
    s = c.getSection("smpp");
    smppHost = s.getString("host" + instanceN);
    smppPort = s.getInt("port" + instanceN);
    s = c.getSection("profiler").getSection("notify");
    agentHost = s.getString("host" + instanceN);
    agentPort = s.getInt("port" + instanceN);
    cacheDir = s.getString("dir" + instanceN);
    s = c.getSection("inman");
    inmanHost = s.getString("host" + instanceN);
    inmanPort = s.getInt("port" + instanceN);
  }

  protected void save(int instanceN, XmlConfig c) throws XmlConfigException {
    XmlConfigSection s = c.getOrCreateSection("admin");
    s.setString("host" + instanceN, adminHost);
    s.setInt("port" + instanceN, adminPort);
    XmlConfigSection core = c.getOrCreateSection("core");
    s = core.getOrCreateSection("performance");
    s.setString("host" + instanceN, corePerfHost);
    s.setInt("port" + instanceN, corePerfPort);

    s = core.getOrCreateSection("smeperformance");
    s.setString("host" + instanceN, smePerfHost);
    s.setInt("port" + instanceN, smePerfPort);

    s = c.getOrCreateSection("MessageStore");
    s.setString("statisticsDir" + instanceN, messageStoreStatsDir);
    s.setString("archiveDir" + instanceN, messageStoreArchDir);

    s = s.getOrCreateSection("LocalStore");
    s.setString("filename" + instanceN, localStoreFilename);
    s.setString("msgidfile" + instanceN, localStoreMessIdFile);

    s = c.getOrCreateSection("smpp");
    s.setString("host" + instanceN, smppHost);
    s.setInt("port" + instanceN, smppPort);

    s = c.getOrCreateSection("profiler").getOrCreateSection("notify");
    s.setString("host" + instanceN, agentHost);
    s.setInt("port" + instanceN, agentPort);
    s.setString("dir" + instanceN, cacheDir);

    s = c.getOrCreateSection("inman");
    s.setString("host" + instanceN, inmanHost);
    s.setInt("port" + instanceN, inmanPort);
  }

  public String getAdminHost() {
    return adminHost;
  }

  public void setAdminHost(String adminHost) throws AdminException {
    vh.checkNotEmpty("adminHost", adminHost);
    this.adminHost = adminHost;
  }

  public int getAdminPort() {
    return adminPort;
  }

  public void setAdminPort(int adminPort) throws AdminException {
    vh.checkPort("adminPort", adminPort);
    this.adminPort = adminPort;
  }

  public String getCorePerfHost() {
    return corePerfHost;
  }

  public void setCorePerfHost(String corePerfHost) throws AdminException {
    vh.checkNotEmpty("corePerfHost", corePerfHost);
    this.corePerfHost = corePerfHost;
  }

  public int getCorePerfPort() {
    return corePerfPort;
  }

  public void setCorePerfPort(int corePerfPort) throws AdminException {
    vh.checkPort("corePerfPort", corePerfPort);
    this.corePerfPort = corePerfPort;
  }

  public String getSmePerfHost() {
    return smePerfHost;
  }

  public void setSmePerfHost(String smePerfHost) throws AdminException {
    vh.checkNotEmpty("smePerfHost", smePerfHost);
    this.smePerfHost = smePerfHost;
  }

  public int getSmePerfPort() {
    return smePerfPort;
  }

  public void setSmePerfPort(int smePerfPort) throws AdminException {
    vh.checkPort("smePerfPort", smePerfPort);
    this.smePerfPort = smePerfPort;
  }

  public String getMessageStoreStatsDir() {
    return messageStoreStatsDir;
  }

  public void setMessageStoreStatsDir(String messageStoreStatsDir) throws AdminException {
    vh.checkNotEmpty("messageStoreStatsDir", messageStoreStatsDir);
    this.messageStoreStatsDir = messageStoreStatsDir;
  }

  public String getMessageStoreArchDir() {
    return messageStoreArchDir;
  }

  public void setMessageStoreArchDir(String messageStoreArchDir) throws AdminException {
    vh.checkNotEmpty("messageStoreArchDir", messageStoreArchDir);
    this.messageStoreArchDir = messageStoreArchDir;
  }

  public String getLocalStoreFilename() {
    return localStoreFilename;
  }

  public void setLocalStoreFilename(String localStoreFilename) throws AdminException {
    vh.checkNotEmpty("localStoreFilename", localStoreFilename);
    this.localStoreFilename = localStoreFilename;
  }

  public String getLocalStoreMessIdFile() {
    return localStoreMessIdFile;
  }

  public void setLocalStoreMessIdFile(String localStoreMessIdFile) throws AdminException {
    vh.checkNotEmpty("localStoreMessIdFile", localStoreMessIdFile);
    this.localStoreMessIdFile = localStoreMessIdFile;
  }

  public String getSmppHost() {
    return smppHost;
  }

  public void setSmppHost(String smppHost) throws AdminException {
    vh.checkNotEmpty("smppHost", smppHost);
    this.smppHost = smppHost;
  }

  public int getSmppPort() {
    return smppPort;
  }

  public void setSmppPort(int smppPort) throws AdminException {
    vh.checkPort("smppPort", smppPort);
    this.smppPort = smppPort;
  }

  public String getAgentHost() {
    return agentHost;
  }

  public void setAgentHost(String agentHost) throws AdminException {
    vh.checkNotEmpty("agentHost", agentHost);
    this.agentHost = agentHost;
  }

  public int getAgentPort() {
    return agentPort;
  }

  public void setAgentPort(int agentPort) throws AdminException {
    vh.checkPort("agentPort", agentPort);
    this.agentPort = agentPort;
  }

  public String getCacheDir() {
    return cacheDir;
  }

  public void setCacheDir(String cacheDir) throws AdminException {
    vh.checkNotEmpty("cacheDir", cacheDir);
    this.cacheDir = cacheDir;
  }

  public String getInmanHost() {
    return inmanHost;
  }

  public void setInmanHost(String inmanHost) throws AdminException {
    vh.checkNotEmpty("inmanHost", inmanHost);
    this.inmanHost = inmanHost;
  }

  public int getInmanPort() {
    return inmanPort;
  }

  public void setInmanPort(int inmanPort) throws AdminException {
    vh.checkPort("inmanPort", inmanPort);
    this.inmanPort = inmanPort;
  }

  public Object clone() throws CloneNotSupportedException {
    InstanceSettings s = (InstanceSettings) super.clone();
    s.adminHost = adminHost;
    s.adminPort = adminPort;
    s.corePerfHost = corePerfHost;
    s.corePerfPort = corePerfPort;
    s.smePerfHost = smePerfHost;
    s.smePerfPort = smePerfPort;
    s.messageStoreStatsDir = messageStoreStatsDir;
    s.messageStoreArchDir = messageStoreArchDir;
    s.localStoreFilename = localStoreFilename;
    s.localStoreMessIdFile = localStoreMessIdFile;
    s.smppHost = smppHost;
    s.smppPort = smppPort;
    s.agentHost = agentHost;
    s.agentPort = agentPort;
    s.cacheDir = cacheDir;
    s.inmanHost = inmanHost;
    s.inmanPort = inmanPort;
    return s;
  }

}
