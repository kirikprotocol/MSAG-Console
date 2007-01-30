package ru.sibinco.smpp.cmb;

import java.util.HashMap;
import java.util.Map;

public class ReportsQueue {

  private static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(ReportsQueue.class);

  private Map waitingReports = new HashMap();

  private static ReportsQueue instance = null;

  private ReportsQueue() {
  }

  public static ReportsQueue getInstance() {
    if (instance == null)
      instance = new ReportsQueue();
    return instance;
  }

  public synchronized void addReport(Report report) {
    Report _old = (Report) waitingReports.put(new Integer(report.getId()), report);
    if (Logger.isDebugEnabled())
      Logger.debug("Added report #" + report.getId() + " for abonent #" + report.getAbonent());
    if (_old != null)
      Logger.warn("Report replaced by ID=" + _old.getId());
  }

  public synchronized Report getReport(int id) {
    Report report = (Report) waitingReports.remove(new Integer(id));
    if (report == null) {
      Logger.warn("Report #" + id + " not found");
      return null;
    } else {
      if (Logger.isDebugEnabled())
        Logger.debug("Got report #" + report.getId() + " for abonent #" + report.getAbonent());
      return report;
    }
  }
}
