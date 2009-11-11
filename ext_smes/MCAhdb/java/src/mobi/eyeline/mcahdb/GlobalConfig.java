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
  private final int eventsStoreRWTimeout;
  private final int eventsStoreROTimeout;

  private final String soapResponseDateFormat;
  private final int soapEventsFetchInterval;

  private final String schedulerExpiredNotifText;
  private final String schedulerStoreDir;
  private final String schedulerErrorText;
  private final String schedulerProfileChangedText;
  private final String schedulerServiceType;
  private final int schedulerExpirationPeriod;
  private final boolean schedulerEnableAlerts;

  private final boolean jmx;
  private final int jmxPort;
  private final String jmxUser;
  private final String jmxPassword;

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
    eventsStoreRWTimeout = eventsStoreSection.getInt("rwTimeout");
    eventsStoreROTimeout = eventsStoreSection.getInt("roTimeout");

    XmlConfigSection soapSection = config.getSection("soap");
    if (soapSection == null)
      throw new ConfigException("'soap' section not found in config.xml");
    soapResponseDateFormat = soapSection.getString("responseDateFormat");
    soapEventsFetchInterval = soapSection.getInt("eventsFetchInterval");

    XmlConfigSection schedulerSection = config.getSection("scheduler");
    if (schedulerSection == null)
      throw new ConfigException(("'scheduler' section not found in config.xml"));
    schedulerExpiredNotifText = schedulerSection.getString("expiredText", null);
    schedulerStoreDir = schedulerSection.getString("storeDir");
    schedulerErrorText = schedulerSection.getString("errorText", null);
    schedulerProfileChangedText = schedulerSection.getString("profileChangedText", null);
    schedulerExpirationPeriod = schedulerSection.getInt("expirationPeriod");
    schedulerServiceType = schedulerSection.getString("serviceType");
    schedulerEnableAlerts = schedulerSection.getBool("enableAlerts", true);

    XmlConfigSection jmx = config.getSection("jmx");
    this.jmx = jmx != null;
    if (this.jmx) {
      this.jmxPort = jmx.getInt("port");
      this.jmxUser = jmx.getString("user");
      this.jmxPassword = jmx.getString("password");
    } else {
      this.jmxPort = 0;
      this.jmxUser = null;
      this.jmxPassword = null;
    }
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

  public int getEventsStoreRWTimeout() {
    return eventsStoreRWTimeout;
  }

  public int getEventsStoreROTimeout() {
    return eventsStoreROTimeout;
  }

  public String getSoapResponseDateFormat() {
    return soapResponseDateFormat;
  }

  public int getSoapEventsFetchInterval() {
    return soapEventsFetchInterval;
  }

  public boolean getSchedulerEnableAlerts() {
    return schedulerEnableAlerts;
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

  public String getSchedulerServiceType() {
    return schedulerServiceType;
  }

  public boolean isJmx() {
    return jmx;
  }

  public int getJmxPort() {
    return jmxPort;
  }

  public String getJmxUser() {
    return jmxUser;
  }

  public String getJmxPassword() {
    return jmxPassword;
  }
}
