package ru.novosoft.smsc.mtsmsme.beans;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.admin.AdminException;

import java.io.*;
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

    backup.removeParamsFromSection("MTSMSme");
    backup.copySectionParamsFromConfig(config, "MTSMSme");

    for (Iterator i = backup.getSectionChildSectionNames("MTSMSme").iterator(); i.hasNext();) {
      String sectionName = (String) i.next();
      backup.removeSection(sectionName);
    }
    for (Iterator i = config.getSectionChildSectionNames("MTSMSme").iterator(); i.hasNext();) {
      String sectionName = (String) i.next();
      backup.copySectionFromConfig(config, sectionName);
    }

    backup.save();
    return message("Changes saved, you should restart MTSM Sme to apply changes");
  }

  protected int reset()
  {
    try {
      getMTSMSmeContext().resetConfig();
    } catch (Throwable e) {
      logger.debug("Couldn't reload MTSM Sme config", e);
      return error("Could not reload MTSM Sme config", e);
    }
    return RESULT_DONE;
  }

  protected int apply()
  {
    int result = RESULT_DONE;
    try {
      logger.debug("Saving ...");
      result = super.save();
      if (result == RESULT_DONE) {
        logger.debug("Apply ...");
        final Config oldConfig = getMTSMSmeContext().loadCurrentConfig();
        result = applyGlobalParams(oldConfig);
      }
    } catch (Throwable e) {
      logger.error("Couldn't save MTSM Sme config", e);
      result = error("Could not save MTSM Sme config", e);
    }
    return result;
  }

  protected int start()
  {
    int result = RESULT_DONE;
    try {
      getAppContext().getHostsManager().startService(getSmeId());
      try { Thread.sleep(5000); } catch (InterruptedException e) {}
    } catch (AdminException e) {
      logger.error("Could not start MTSM Sme", e);
      result = error("Could not start MTSM Sme", e);
    }
    return result;
  }

  protected int stop()
  {
    int result = RESULT_DONE;
    try {
      getAppContext().getHostsManager().shutdownService(getSmeId());
    } catch (AdminException e) {
      logger.error("Could not stop MTSM Sme", e);
      result = error("Could not stop MTSM Sme", e);
    }
    return result;
  }

}
