package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;

import java.io.*;
import java.util.Iterator;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.04.2004
 * Time: 14:59:49
 * To change this template use Options | File Templates.
 */
public class Index extends IndexProperties
{
  private int applyGlobalParams(final Config oldConfig)
      throws Config.WrongParamTypeException, IOException, NullPointerException, CloneNotSupportedException
  {
    Config backup = (Config) oldConfig.clone();
    final Config config = getConfig();

    backup.removeParamsFromSection("MCISme");
    backup.copySectionParamsFromConfig(config, "MCISme");

    backup.removeSection("StartupLoader");
    backup.copySectionFromConfig(config, "StartupLoader");

    for (Iterator i = backup.getSectionChildSectionNames("MCISme").iterator(); i.hasNext();) {
      String sectionName = (String) i.next();
      backup.removeSection(sectionName);
    }
    for (Iterator i = config.getSectionChildSectionNames("MCISme").iterator(); i.hasNext();) {
      String sectionName = (String) i.next();
      backup.copySectionFromConfig(config, sectionName);
    }

    backup.save();

    getMCISmeContext().setChangedOptions(false);
    getMCISmeContext().setChangedDrivers(false);
    getMCISmeContext().setChangedTemplates(false);
    getMCISmeContext().reloadDataSource(oldConfig, config);
    if (getMCISmeContext().getDataSource() == null)
      warning("Invalid JDBC parameters");

    saveMCIProfOptions(config);

    return message("Changes saved, you should restart MCI Sme to apply changes");
  }

  protected int reset()
  {
    try {
      getMCISmeContext().resetConfig();
      saveMCIProfOptions(getConfig());
      getMCISmeContext().setChangedOptions(false);
      getMCISmeContext().setChangedDrivers(false);
      getMCISmeContext().setChangedTemplates(false);
    } catch (Throwable e) {
      logger.debug("Couldn't reload MCI Sme config", e);
      return error("Could not reload MCI Sme config", e);
    }
    return RESULT_DONE;
  }

  protected int apply()
  {
    int result = RESULT_DONE;
    try {
      logger.debug("Apply ...");
      final Config oldConfig = getMCISmeContext().loadCurrentConfig();
      result = applyGlobalParams(oldConfig);
    } catch (Throwable e) {
      logger.error("Couldn't save MCI Sme config", e);
      result = error("Could not save MCI Sme config", e);
    }
    return result;
  }

  protected int start()
  {
    int result = RESULT_DONE;
    if (isToStart("service"))
    {
      try {
        getAppContext().getHostsManager().startService(getSmeId());
        try { Thread.sleep(5000); } catch (InterruptedException e) {}
      } catch (AdminException e) {
        logger.error("Could not start MCI Sme", e);
        result = error("Could not start MCI Sme", e);
      }
    }
    else if (isToStart("profiler"))
    {
      try {
        getAppContext().getHostsManager().startService(getProfilerSmeId());
        try { Thread.sleep(5000); } catch (InterruptedException e) {}
      } catch (AdminException e) {
        logger.error("Could not start MCI Profiler", e);
        result = error("Could not start MCI Profiler", e);
      }
    }
    return result;
  }

  protected int stop()
  {
    int result = RESULT_DONE;
    if (isToStart("service"))
    {
      try {
        getAppContext().getHostsManager().shutdownService(getSmeId());
      } catch (AdminException e) {
        logger.error("Could not stop MCI Sme", e);
        result = error("Could not stop MCI Sme", e);
      }
    }
    else if (isToStart("profiler"))
    {
      try {
        getAppContext().getHostsManager().shutdownService(getProfilerSmeId());
      } catch (AdminException e) {
        logger.error("Could not stop MCI Profiler", e);
        result = error("Could not stop MCI Profiler", e);
      }
    }
    return result;
  }

  private void savePropertiesFile(String location, String fileName, Properties properties, String comment)
      throws IOException
  {
    File file = new File(location+'/'+fileName);
    if (file.exists()) file.delete();
    file.createNewFile();

    OutputStream os = null;
    try {
      os = new FileOutputStream(file);
      properties.store(os, comment);
    } catch(IOException e) {
      throw e;
    } finally {
      if (os != null) { os.flush(); os.close(); }
    }
  }
  private void saveMCIProfOptions(Config config) throws IOException
  {
    try
    {
      final String location = config.getString(MCI_PROF_LOCATION_PARAM);
      if (location == null || location.trim().length() <= 0) return;

      Properties dsProperties = new Properties();
      dsProperties.setProperty("jdbc.driver"  , config.getString("MCISme.DataSource.jdbc.driver"));
      dsProperties.setProperty("jdbc.source"  , config.getString("MCISme.DataSource.jdbc.source"));
      dsProperties.setProperty("jdbc.user"    , config.getString("MCISme.DataSource.dbUserName"));
      dsProperties.setProperty("jdbc.password", config.getString("MCISme.DataSource.dbUserPassword"));
      dsProperties.setProperty("Pool.Size"                 , "5");
      dsProperties.setProperty("Pool.getConnection.timeout", "30");
      dsProperties.setProperty("Pool.getConnection.check"  , "true");
      dsProperties.setProperty("Pool.fill.factor"          , "0");
      savePropertiesFile(location, MCI_PROF_DS_FILE, dsProperties, MCI_PROF_DS_COMMENT);

      Properties mscProperties = new Properties();
      mscProperties.setProperty("MSC.host"        , config.getString("MCISme.MSC.host"));
      mscProperties.setProperty("MSC.port"        , ""+config.getInt("MCISme.MSC.port"));
      mscProperties.setProperty("MSC.usercode"    , config.getString("MCISme.MSC.usercode"));
      mscProperties.setProperty("MSC.userpassword", config.getString("MCISme.MSC.userpassword"));
      mscProperties.setProperty("MSC.nvtIODevice" , config.getString("MCISme.MSC.nvtIODevice"));
      mscProperties.setProperty("MSC.voicemail"   , config.getString("MCISme.MSC.voicemail"));
      mscProperties.setProperty("MSC.mcisme"      , config.getString("MCISme.MSC.mcisme"));
      String mcismeReasons = "";
      if (config.getBool("MCISme.Reasons.Busy.inform"))          mcismeReasons += "B";
      if (config.getBool("MCISme.Reasons.Absent.inform"))        mcismeReasons += "A";
      if (config.getBool("MCISme.Reasons.NoReply.inform"))       mcismeReasons += "N";
      if (config.getBool("MCISme.Reasons.Unconditional.inform")) mcismeReasons += "U";
      mscProperties.setProperty("MSC.mcisme.reasons", mcismeReasons);
      mscProperties.setProperty("MSC.mcisme.strategy", ""+config.getInt("MCISme.Reasons.strategy"));
      savePropertiesFile(location, MCI_PROF_MSC_FILE, mscProperties, MCI_PROF_MSC_COMMENT);

      Properties mtfProperties = new Properties();
      mtfProperties.setProperty("defaultInform", ""+config.getBool("MCISme.defaultInform"));
      mtfProperties.setProperty("defaultInformId", ""+config.getInt(INFORM_TEMPLATES_SECTION_NAME+".default"));
      for (Iterator i = config.getSectionChildShortSectionNames(INFORM_TEMPLATES_SECTION_NAME).iterator(); i.hasNext();) {
        String templateName = (String)i.next();
        int id = config.getInt(INFORM_TEMPLATES_SECTION_NAME+'.'+templateName+".id");
        mtfProperties.setProperty("informTemplate."+id, templateName);
      }
      mtfProperties.setProperty("defaultNotify", ""+config.getBool("MCISme.defaultNotify"));
      mtfProperties.setProperty("defaultNotifyId", ""+config.getInt(NOTIFY_TEMPLATES_SECTION_NAME+".default"));
      for (Iterator i = config.getSectionChildShortSectionNames(NOTIFY_TEMPLATES_SECTION_NAME).iterator(); i.hasNext();) {
        String templateName = (String)i.next();
        int id = config.getInt(NOTIFY_TEMPLATES_SECTION_NAME+'.'+templateName+".id");
        mtfProperties.setProperty("notifyTemplate."+id, templateName);
      }
      savePropertiesFile(location, MCI_PROF_MTF_FILE, mtfProperties, MCI_PROF_MTF_COMMENT);

    } catch (IOException e) {
      throw e;
    } catch (Exception e) {
      logger.warn("Failed to save MCIProf options", e);
    }

  }

}
