package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.mcisme.backend.CountersSet;

import java.io.IOException;
import java.util.Iterator;

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

    //InfoSme - root
    backup.removeParamsFromSection("MCISme");
    backup.copySectionParamsFromConfig(config, "MCISme");

    //StartupLoader
    backup.removeSection("StartupLoader");
    backup.copySectionFromConfig(config, "StartupLoader");

    //others
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
    getMCISmeContext().reloadDataSource(oldConfig, config);
    if (getMCISmeContext().getDataSource() == null)
      warning("Invalid JDBC parameters");

    return message("Changes saved, you should restart MCI Sme to apply changes");
  }

  protected int reset()
  {
    try {
      getMCISmeContext().resetConfig();
      getMCISmeContext().setChangedOptions(false);
      getMCISmeContext().setChangedDrivers(false);
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
    try {
      getAppContext().getHostsManager().startService(getSmeId());
      try { // из-за долгого старта MCISme
        Thread.sleep(5000);
      } catch (InterruptedException e) {
        //do nothing
      }
    } catch (AdminException e) {
      logger.error("Could not start MCI Sme", e);
      result = error("Could not start MCI Sme", e);
    }
    return result;
  }

  protected int stop()
  {
    int result = RESULT_DONE;
    try {
      getAppContext().getHostsManager().shutdownService(getSmeId());
    } catch (AdminException e) {
      logger.error("Could not stop MCI Sme", e);
      result = error("Could not stop MCI Sme", e);
    }
    return result;
  }

}
