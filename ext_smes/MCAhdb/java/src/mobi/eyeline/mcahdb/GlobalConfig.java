package mobi.eyeline.mcahdb;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;
import com.eyeline.utils.config.ConfigException;

import java.io.File;

import mobi.eyeline.mcahdb.engine.journal.JournalsProcessor;
import mobi.eyeline.mcahdb.engine.event.EventStore;
import mobi.eyeline.mcahdb.engine.scheduler.Scheduler;
import mobi.eyeline.mcahdb.soap.SoapConfig;

/**
 * User: artem
 * Date: 06.08.2008
 */

public class GlobalConfig implements EventStore.Config,
                                     JournalsProcessor.Config,
                                     Scheduler.Config, SoapConfig {

  private final int journalsCheckInterval;
  private final String journalsDir;
  private final String journalsArchivesDir;
  private final String eventsStoreDir;
  private final String soapResponseDateFormat;
  private final int soapEventsFetchInterval;

  private final String schedulerExpiredNotifText;
  private final String schedulerStoreDir;
  private final String schedulerErrorText;
  private final String schedulerProfileChangedText;
  private final int schedulerExpirationPeriod;

  public GlobalConfig() throws ConfigException {
    XmlConfig config = new XmlConfig();

    config.load(new File("conf", "config.xml"));

    journalsCheckInterval = config.getInt("journalsCheckInterval");

    XmlConfigSection journalsSection = config.getSection("journalStore");
    if (journalsSection == null)
      throw new ConfigException("'journalsStore' section not found in config.xml");
    journalsDir = journalsSection.getString("journalsDir");
    journalsArchivesDir = journalsSection.getString("archivesDir");

    XmlConfigSection eventsStoreSection = config.getSection("eventsStore");
    if (eventsStoreSection == null)
      throw new ConfigException("'eventsStore' section not found in config.xml");
    eventsStoreDir = eventsStoreSection.getString("storeDir");

    XmlConfigSection soapSection = config.getSection("soap");
    if (soapSection == null)
      throw new ConfigException("'soap' section not found in config.xml");
    soapResponseDateFormat = soapSection.getString("responseDateFormat");
    soapEventsFetchInterval = soapSection.getInt("eventsFetchInterval");

    XmlConfigSection schedulerSection = config.getSection("scheduler");
    if (schedulerSection == null)
      throw new ConfigException(("'scheduler' section not found in config.xml"));
    schedulerExpiredNotifText = schedulerSection.getString("expiredText");
    schedulerStoreDir = schedulerSection.getString("storeDir");
    schedulerErrorText = schedulerSection.getString("errorText");
    schedulerProfileChangedText = schedulerSection.getString("profileChangedText");
    schedulerExpirationPeriod = schedulerSection.getInt("expirationPeriod");
  }

  public int getJournalsCheckInterval() {
    return journalsCheckInterval;
  }

  public String getJournalsStoreDir() {
    return journalsDir;
  }

  public String getJournalsArchivesDir() {
    return journalsArchivesDir;
  }

  public String getEventsStoreDir() {
    return eventsStoreDir;
  }

  public String getSoapResponseDateFormat() {
    return soapResponseDateFormat;
  }

  public int getSoapEventsFetchInterval() {
    return soapEventsFetchInterval;
  }

  public String getSchedulerExpiredNotifText() {
    return schedulerExpiredNotifText;
  }

  public String getSchedulerStoreDir() {
    return schedulerStoreDir;
  }

  public String getSchedulerErrorText() {
    return schedulerErrorText;
  }

  public String getSchedulerProfileChangedText() {
    return schedulerProfileChangedText;
  }

  public int getSchedulerExpirationPeriod() {
    return schedulerExpirationPeriod;
  }
}
