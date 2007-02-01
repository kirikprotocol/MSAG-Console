package ru.sibinco.smpp.cmb;

import com.logica.smpp.Data;
import ru.sibinco.smpp.*;

import java.util.LinkedList;
import java.util.List;
import java.util.Properties;
import java.io.IOException;

public class ReportSender implements ReiterationTask, MessageStatusListener {

  private static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(ReportSender.class);

  private ReportsQueue rQueue = ReportsQueue.getInstance();
  private List reports = new LinkedList();
  private boolean registered = false;
  private ConstraintManager constraintManager = null;

  public void init(String fileName) throws InitializationException {
    Properties config = new Properties();
    try {
      config.load(this.getClass().getClassLoader().getResourceAsStream(fileName));
    } catch (IOException e) {
      throw new InitializationException("Could not load config for "+getName()+" from "+fileName, e);
    }
    if (config.getProperty("constraint.manager.class") != null) {
      try {
        constraintManager = (ConstraintManager)Class.forName(config.getProperty("constraint.manager.class")).newInstance();
      } catch (Exception e) {
        throw new InitializationException("Could not initialize ConstraintManager "+config.getProperty("constraint.manager.class")+" for "+getName(), e);
      }
    } else {
      constraintManager = new ConstraintManagerImpl();
    }
  }

  public String getName() {
    return "ReportSender";
  }

  public long getReiterationPeriod() {
    return 100;
  }

  public long getSendInterval() {
    return 100;
  }

  public void clearResources() {
  }

  public boolean execute(OutgoingQueueProxy outgoingQueueProxy) throws ReiterationTaskException {
    if (!registered) {
      outgoingQueueProxy.registerMessageStatusListener(this);
      registered = true;
    }
    if (!reports.isEmpty()) {
      MessageData report;
      synchronized (reports) {
        report = (MessageData) reports.remove(0);
      }
      if (report != null) {
        outgoingQueueProxy.addOutgoingObject(new Response(Response.TYPE_ARRAY, new MessageData[]{report}));
        if (Logger.isDebugEnabled())
          Logger.debug(getName() + " sending report to abonent #" + report.getDestinationAddress()+", msg: "+report.getMessageString());
        return true;
      } else
        return false;
    } else
      return false;
  }

  public void statusChanged(int id, int status) {
    Report report = rQueue.getReport(id);
    if (report != null) {
      synchronized (reports) {
        reports.add(status == Data.ESME_ROK ? report.getSuccessReport() : report.getFailedReport());
      }
      if (Logger.isDebugEnabled())
        Logger.debug(getName() + " process report #" + report.getId() + " for abonent #" + report.getAbonent() + ", notification " + (status == Data.ESME_ROK ? "delivered" : "failed"));
      if (status != Data.ESME_ROK)
        try {
          constraintManager.unregisterUsage(report.getAbonent());
        } catch (CheckConstraintsException e) {
          Logger.error(getName() + " could not reduce usages for abonent #" + report.getAbonent(), e);
        }
    }
  }
}
