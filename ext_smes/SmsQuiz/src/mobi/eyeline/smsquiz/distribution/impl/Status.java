package mobi.eyeline.smsquiz.distribution.impl;

import org.apache.log4j.Logger;

import java.io.*;

/**
 * author: alkhal
 */
public class Status {

  private Logger logger = Logger.getLogger(Status.class);

  public static enum DistrStatus {
    INPROGRESS, FINISHED
  }

  private DistrStatus status;
  private Runnable origTask;
  private long creationDate;

  private String errorFile;

  private boolean executed = false;

  public Status(Runnable origTask, String errorFile) {
    creationDate = System.currentTimeMillis();
    status = DistrStatus.INPROGRESS;
    this.origTask = origTask;
    this.errorFile = errorFile;
  }

  public void start() {
    executed = true;
    new Thread(origTask).start();
  }

  public long getCreationDate() {
    return creationDate;
  }

  public boolean isExecuted() {
    return executed;
  }

  public DistrStatus getStatus() {
    return status;
  }

  public void setStatus(DistrStatus status) {
    this.status = status;
  }

}
