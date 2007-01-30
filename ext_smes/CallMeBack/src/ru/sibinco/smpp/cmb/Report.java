package ru.sibinco.smpp.cmb;

import ru.sibinco.smpp.MessageData;

public class Report {
  private int id;
  private MessageData successReport;
  private MessageData failedReport;

  public Report(int id, MessageData successReport, MessageData failedReport) {
    this.id = id;
    this.successReport = successReport;
    this.failedReport = failedReport;
  }

  public int getId() {
    return id;
  }

  public String getAbonent() {
    return successReport.getDestinationAddress();
  }

  public MessageData getSuccessReport() {
    return successReport;
  }

  public MessageData getFailedReport() {
    return failedReport;
  }
}
