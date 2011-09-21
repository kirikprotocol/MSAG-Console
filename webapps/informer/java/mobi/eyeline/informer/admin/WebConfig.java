package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.archive.ArchiveSettings;
import mobi.eyeline.informer.admin.cdr.CdrSettings;
import mobi.eyeline.informer.admin.contentprovider.CpFileFormat;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import mobi.eyeline.informer.admin.siebel.SiebelSettings;
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

    XmlConfigSection javamail = config.getOrCreateSection("javamail");

    Properties props = settings.getNotificationSettings().getMailProperties();
    for (Object s : props.keySet()) {
      javamail.addParam(new XmlConfigParam((String) s, (String) props.get(s), XmlConfigParam.Type.STRING));
    }

    XmlConfigSection sms = config.getSection("sms");
    sms.addParam(new XmlConfigParam("senderAddress", settings.getNotificationSettings().getSmsSenderAddress().getSimpleAddress(), XmlConfigParam.Type.STRING));

    XmlConfigSection notificationTemplates = config.getSection("notificationTemplates");
    props = settings.getNotificationSettings().getNotificationTemplates();
    for (Object s : props.keySet()) {
      notificationTemplates.addParam(new XmlConfigParam((String) s, (String) props.get(s), XmlConfigParam.Type.STRING));
    }

    config.addSection(javamail);

    XmlConfigSection siebel = config.getOrCreateSection("siebel");
    props = settings.getSiebelSettings().getAllProperties();
    for (Object s : props.keySet()) {
      siebel.addParam(new XmlConfigParam((String) s, (String) props.get(s), XmlConfigParam.Type.STRING));
    }
    config.addSection(siebel);

    XmlConfigSection cdr = config.getOrCreateSection("cdr");
    settings.getCdrSettings().save(cdr);

    XmlConfigSection deliveries = config.getOrCreateSection("deliveries");
    deliveries.setBool("allowUssdPush", settings.isAllowUssdPushDeliveries());

    ArchiveSettings archiveSettings = settings.getArchiveSettings();

    XmlConfigSection s = config.getOrCreateSection("archive");
    if(archiveSettings.getResultsDir() != null) {
      s.setString("resultsDir", archiveSettings.getResultsDir());
    }
    if(archiveSettings.getRequestsDir() != null) {
      s.setString("requestsDir", archiveSettings.getRequestsDir());
    }
    s.setInt("chunkSize", archiveSettings.getChunkSize());
    s.setInt("executorsSize", archiveSettings.getExecutorsSize());

    XmlConfigSection cp = config.getOrCreateSection("contentProvider");
    cp.clear();
    if(settings.getCpFileFormat() != null) {
      cp.setString("fileFormat", settings.getCpFileFormat().toString());
    }
    if(settings.getCpGroupSize() != 0) {
      cp.setInt("groupSize", settings.getCpGroupSize());
    }
    if(settings.getCpMaxTimeSec() != 0 ) {
      cp.setInt("maxTimeSec", settings.getCpMaxTimeSec());
    }

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
      settings.setMonitoringDir(system.getString("monitoring dir"));
      settings.setWorkDir(system.getString("work dir"));


      XmlConfigSection javamail = webconfig.getSection("javamail");
      Properties jmp = javamail.toProperties("", null);

      XmlConfigSection sms = webconfig.getSection("sms");
      Address smsAddress = new Address(sms.getString("senderAddress"));

      XmlConfigSection notificationTemplates = webconfig.getSection("notificationTemplates");
      Properties nt = notificationTemplates.toProperties("", null);

      settings.setNotificationSettings(new NotificationSettings(jmp, nt, smsAddress));

      XmlConfigSection siebel = webconfig.getSection("siebel");
      settings.setSiebelSettings(new SiebelSettings(siebel.toProperties("", null)));

      String fF = null;
      if(webconfig.containsSection("contentProvider")) {
        XmlConfigSection cp = webconfig.getSection("contentProvider");
        fF = cp.getString("fileFormat", null);
        int gS = cp.getInt("groupSize", 1);
        if(gS == 0) {
          gS = 1;
        }
        settings.setCpGroupSize(gS);
        int mT = cp.getInt("maxTimeSec", 0);
        settings.setCpMaxTimeSec(mT);
      }

      CpFileFormat cpFileFormat;
      try{
        cpFileFormat = (fF == null || fF.length() == 0 ? CpFileFormat.EYELINE : CpFileFormat.valueOf(fF));

      }catch (IllegalArgumentException e) {
        throw new WebConfigException("invalid_cp_file_format", fF);
      }
      settings.setCpFileFormat(cpFileFormat);

      XmlConfigSection cdr = webconfig.getSection("cdr");
      CdrSettings cdrSettings = new CdrSettings();
      cdrSettings.load(cdr);
      settings.setCdrSettings(cdrSettings);

      XmlConfigSection deliveries = webconfig.getSection("deliveries");
      settings.setAllowUssdPushDeliveries(deliveries.getBool("allowUssdPush", false));

      ArchiveSettings archiveSettings = new ArchiveSettings();
      if(webconfig.containsSection("archive")) {
        XmlConfigSection archive = webconfig.getSection("archive");
        archiveSettings.setResultsDir(archive.getString("resultsDir", null));
        archiveSettings.setRequestsDir(archive.getString("requestsDir", null));
        archiveSettings.setChunkSize(archive.getInt("chunkSize", 10));
        archiveSettings.setExecutorsSize(archive.getInt("executorsSize", 10));
      }
      settings.setArchiveSettings(archiveSettings);
      return settings;
    }
    catch (XmlConfigException e) {
      throw new WebConfigException("invalid_config", e);
    }
  }
}
