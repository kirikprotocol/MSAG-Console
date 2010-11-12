package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Properties;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.11.2010
 * Time: 15:09:38
 */
class WebConfig implements ManagedConfigFile<WebConfigSettings> {

  public void save(InputStream oldFile, OutputStream newFile, WebConfigSettings settings) throws Exception {
    XmlConfig config = new XmlConfig();
    config.load(oldFile);

    XmlConfigSection  javamail = config.getOrCreateSection("javamail");

    Properties props = settings.getJavaMailProperties();
    for(Object s : props.keySet()) {
      javamail.addParam(new XmlConfigParam((String)s,(String)props.get(s),XmlConfigParam.Type.STRING));
    }

    XmlConfigSection sms = config.getSection("sms");
    sms.addParam(new XmlConfigParam("senderAddress",settings.getSmsSenderAddress().getSimpleAddress(),XmlConfigParam.Type.STRING));

    XmlConfigSection notificationTemplates=config.getSection("notificationTemplates");
    props = settings.getNotificationTemplates();
    for(Object s : props.keySet()) {
      notificationTemplates.addParam(new XmlConfigParam((String)s,(String)props.get(s),XmlConfigParam.Type.STRING));
    }

    config.addSection(javamail);    
    config.save(newFile);
  }

  public WebConfigSettings load(InputStream is) throws Exception {
    try {
    XmlConfig webconfig = new XmlConfig();
    webconfig.load(is);

    WebConfigSettings settings = new WebConfigSettings();

      String installationTypeStr = webconfig.getSection("installation").getString("type", "single");
      if (installationTypeStr.equalsIgnoreCase("hs"))
        settings.setInstallationType(InstallationType.HS);
      else if (installationTypeStr.equalsIgnoreCase("ha"))
        settings.setInstallationType(InstallationType.HA);
      else if (installationTypeStr.equalsIgnoreCase("single"))
        settings.setInstallationType(InstallationType.SINGLE);
      else
        throw new WebConfigException("unknown_inst_type", installationTypeStr);


      XmlConfigSection daemon = webconfig.getSection("daemon");
      settings.setDaemonHost(daemon.getString("host"));
      settings.setDaemonPort(daemon.getInt("port"));

      XmlConfigSection installation = webconfig.getSection("installation");
      File mirrorPath = new File(installation.getString("mirrorpath"));
      settings.setAppMirrorDirs(new File[]{mirrorPath});

      XmlConfigSection nodes = webconfig.getSection("nodes");
      Collection<String> hosts = new ArrayList<String>();
      for (XmlConfigSection nodeSection : nodes.sections()) {
        hosts.add(nodeSection.getName());
      }
      settings.setHSDaemonHosts(hosts);

      XmlConfigSection system = webconfig.getSection("system");
      settings.setJournalDir(system.getString("journal dir"));

      XmlConfigSection javamail = webconfig.getSection("javamail");
      settings.setJavaMailProperties(javamail.toProperties("",null));

      XmlConfigSection sms = webconfig.getSection("sms");
      settings.setSmsSenderAddress(new Address(sms.getString("senderAddress")));

      XmlConfigSection notificationTemplates =webconfig.getSection("notificationTemplates");
      settings.setNotificationTemplates(notificationTemplates.toProperties("",null));

      return settings;
    }
    catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }
}
