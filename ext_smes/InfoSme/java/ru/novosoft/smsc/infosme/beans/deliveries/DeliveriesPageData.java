package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.MultiTask;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

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

  private final SMSCAppContext appContext;
  private final InfoSmeContext infoSmeContext;

  // task attributes
  public String activeTaskSubject;
  public String oldActiveTaskSubject;
  public String name;
  public boolean transliterate;
  public boolean retryOnFail;
  public String retryTime;
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
  public String sourceAddress;
  public String errorStr;

  public boolean splitDeliveriesFile = true;

  // process file progress attributes
  public int recordsProcessed;
  public int subjectsFound;
  public int unrecognized;
  public int inblackList;
  public int splitDeliveriesFileStatus;

  // deliveries generation progress attributes
  public Map deliveriesGenProgr;
  public int deliveriesGenStatus;

  // Data
  private File deliveriesFile;
  private HashMap inputFiles;
  private MultiTask task;

  public DeliveriesPageData(SMSCAppContext appContext, InfoSmeContext infoSmeContext) {
    this.appContext = appContext;
    this.infoSmeContext = infoSmeContext;
    clear();
  }

  public final void clear() {
    activeTaskSubject = null;
    oldActiveTaskSubject = null;
    name = null;
    transliterate = false;
    retryOnFail = false;
    retryTime = null;
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
    flash = false;

    deliveriesGenProgr = null;
    deliveriesGenStatus = STATUS_INITIALIZATION;

    recordsProcessed = 0;
    subjectsFound = 0;
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
        return STATUS_STR_ERR + errorStr;
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

  public HashMap getInputFiles() {
    return inputFiles;
  }

  public void setInputFiles(HashMap inputFiles) {
    this.inputFiles = inputFiles;
  }

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
}
