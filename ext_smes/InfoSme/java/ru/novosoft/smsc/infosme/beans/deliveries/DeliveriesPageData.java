package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.MultiTask;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataSource;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyQuery;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataItem;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;

import java.io.File;
import java.util.*;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class DeliveriesPageData {

  public static final String ABONENTS_FILE_PARAM = "abonentsFile";

  public static final int STATUS_INITIALIZATION = 0;
  public static final int STATUS_PROCESSING = 1;
  public static final int STATUS_DONE = 2;
  public static final int STATUS_ERROR = 3;
  public static final int STATUS_CANCELED = 4;

  private final static String STATUS_STR_GENERATING = "Processing...";
  private final static String STATUS_STR_INIT       = "Initialization...";
  private final static String STATUS_STR_DONE       = "Finished";
  private final static String STATUS_STR_CANCELED   = "Canceled";
  private final static String STATUS_STR_ERR        = "Error - ";

  static String PAGE_DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";
  static String PAGE_TIME_FORMAT = "HH:mm:ss";

  private final SMSCAppContext appContext;
  private final InfoSmeContext infoSmeContext;

  // task attributes
  public int activeTaskRegionId;
  public int oldActiveTaskRegionId;
  public String name;
  public boolean transliterate;
  public boolean transactionMode;
  public boolean useDataSm;
  public int useUssdPush = -1;
  public boolean retryOnFail;
  public String endDate;
  public String startDate;
  public String validityPeriod;
  public String validityDate;
  public String activePeriodStart;
  public String activePeriodEnd;
  public String[] activeWeekDays;
  public String text;
  public String recondsNumber;
  public boolean flash;
  public boolean secret;
  public String secretText;
  public boolean secretFlash;
  public String testSmsAddress;
  public String sourceAddress;
  public String errorStr;
  public String retryPolicy;
  public User owner;

  public boolean splitDeliveriesFile = true;
  public boolean useSameSettingsForAllRegions = true;
  public boolean deliveriesFileContainsText = true;

  // process file progress attributes
  public int recordsProcessed;
  public int regionsFound;
  public int unrecognized;
  public int inblackList;
  public int splitDeliveriesFileStatus;
  public String splitDeliveriesFileStatusStr;

  // deliveries generation progress attributes
  public Map deliveriesGenProgr;
  public int deliveriesGenStatus;

  // Data
  private File deliveriesFile;
//  private HashMap inputFiles;
  private MultiTask task;

  public DeliveriesPageData(SMSCAppContext appContext, InfoSmeContext infoSmeContext) {
    this.appContext = appContext;
    this.infoSmeContext = infoSmeContext;
    clear();
  }

  public final void clear() {
    activeTaskRegionId = -1;
    oldActiveTaskRegionId = -1;
    name = null;
    splitDeliveriesFile=true;
    useSameSettingsForAllRegions=true;
    transliterate = false;
    transactionMode = false;
    useDataSm = false;
    if ( infoSmeContext.getInfoSmeConfig().hasUssdPushFeature() ) {
        useUssdPush = 0;
    } else {
        useUssdPush = -1;
    }
    retryOnFail = false;
    endDate = null;
    startDate = null;
    validityPeriod = null;
    validityDate = null;
    activePeriodStart = null;
    activePeriodEnd = null;
    activeWeekDays = null;
    text = null;
    recondsNumber = null;
    sourceAddress = null;
    retryPolicy = null;
    flash = false;
    secret = false;
    secretFlash = false;
    secretText = null;

    deliveriesGenProgr = null;
    deliveriesGenStatus = STATUS_INITIALIZATION;

    recordsProcessed = 0;
    regionsFound = 0;
    unrecognized = 0;
    inblackList = 0;
    splitDeliveriesFileStatus = STATUS_INITIALIZATION;

    deliveriesFile = null;
    task = null;
  }

  public String getDeliveriesGenStatus() {
    switch(deliveriesGenStatus) {
      case STATUS_INITIALIZATION:
        return STATUS_STR_INIT;
      case STATUS_PROCESSING:
        return STATUS_STR_GENERATING;
      case STATUS_DONE:
        return STATUS_STR_DONE;
      case STATUS_CANCELED:
        return STATUS_STR_CANCELED;
      default:
        return STATUS_STR_ERR + errorStr;
    }
  }

  public String getProcessFileStatus() {
    switch(splitDeliveriesFileStatus) {
      case STATUS_INITIALIZATION:
        return STATUS_STR_INIT;
      case STATUS_PROCESSING:
        return STATUS_STR_GENERATING;
      case STATUS_DONE:
        return STATUS_STR_DONE;
      case STATUS_CANCELED:
        return STATUS_STR_CANCELED;
      default:
        return STATUS_STR_ERR + splitDeliveriesFileStatusStr;
    }
  }

  public SMSCAppContext getAppContext() {
    return appContext;
  }

  public InfoSmeContext getInfoSmeContext() {
    return infoSmeContext;
  }

  public File getDeliveriesFile() {
    return deliveriesFile;
  }

  public void setDeliveriesFile(File deliveriesFile) {
    this.deliveriesFile = deliveriesFile;
  }

//  public HashMap getInputFiles() {
//    return inputFiles;
//  }
//
//  public void setInputFiles(HashMap inputFiles) {
//    this.inputFiles = inputFiles;
//  }

  public MultiTask getTask() {
    return task;
  }

  public void setTask(MultiTask task) {
    this.task = task;
  }

  public boolean isSplitDeliveriesFile() {
    return splitDeliveriesFile;
  }

  public void setSplitDeliveriesFile(boolean splitDeliveriesFile) {
    this.splitDeliveriesFile = splitDeliveriesFile;
  }

  public boolean isUseSameSettingsForAllRegions() {
    return useSameSettingsForAllRegions;
  }

  public void setUseSameSettingsForAllRegions(boolean useSameSettingsForAllRegions) {
    this.useSameSettingsForAllRegions = useSameSettingsForAllRegions;
  }

  public List getRetryPolicies() throws AdminException {
    QueryResultSet rs = new RetryPolicyDataSource().query(infoSmeContext.getInfoSmeConfig(), new RetryPolicyQuery(1000, "name", 0));
    List result = new ArrayList(rs.size() + 1);
    for (int i=0; i<rs.size(); i++) {
      RetryPolicyDataItem item = (RetryPolicyDataItem)rs.get(i);
      result.add(item.getName());
    }
    return result;
  }
}
