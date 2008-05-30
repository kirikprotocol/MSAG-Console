package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.PDU;

import java.util.*;


public class OutgoingQueue implements OutgoingQueueProxy {
  /**
   * Logger.
   */
  private final static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(OutgoingQueue.class);
  /**
   * List of tasks with start time, for tracking submit timeouts.
   */
  private List sendTasks;
  /**
   * List of tasks with start time, for messages that should be retried.
   */
  private SortedSet retryTasks;
  /**
   * After this period the submit will be rescheduled.
   */
  private long submitTimeout = 30000L;
  /**
   * After this retry abonentRequest will be deleted.
   */
  private long maxRetry = 1;
  /**
   * After this period the submit will be retryed.
   */
  private long retryPeriod = 30000L;
  /**
   * Outgoing queue.
   */
  private List outQueue = new LinkedList();
  /**
   * Monitor for notifying sender.
   */
  private Object sendNotifyMonitor = new Object();
  /**
   * Object for selecting text slicer.
   */

  /**
   * Message status changed notifier & monitor
   */
  private MessageStatusChangedNotifier messageStatusChangedNotifier = null;
  private MessageSender messageSender;

  public OutgoingQueue(long submitTimeout, long maxRetry, long retryPeriod) {
    if (submitTimeout < 1000)
      if (Logger.isInfoEnabled())
        Logger.info("Submit timeout too short (" + submitTimeout + "), should be 1000 or more. Use default submit timeout: 30000 millis.");
      else {
        if (Logger.isInfoEnabled()) Logger.info("Use " + submitTimeout + " submit timeout.");
        this.submitTimeout = submitTimeout;
      }
    if (retryPeriod < 1000)
      if (Logger.isInfoEnabled())
        Logger.info("Retry period too short (" + retryPeriod + "), should be 1000 or more. Use default retry period: 30000 millis.");
      else {
        if (Logger.isInfoEnabled()) Logger.info("Use " + retryPeriod + " retry period.");
        this.retryPeriod = retryPeriod;
      }
    if (maxRetry < 1)
      if (Logger.isInfoEnabled())
        Logger.info("Invalid max retries (" + maxRetry + "). Use default max retries: 1.");
      else {
        if (Logger.isInfoEnabled()) Logger.info("Use " + maxRetry + " max retries.");
        this.maxRetry = maxRetry;
      }
    sendTasks = new LinkedList();
    retryTasks = new TreeSet(new RetryTask());
    messageStatusChangedNotifier = new MessageStatusChangedNotifier();
    messageStatusChangedNotifier.startService();
    if (Logger.isInfoEnabled()) Logger.info("OutgoingQueue created.");
  }

  public synchronized OutgoingObject getOutgoingObject() { // todo ulock incoming queue by outgoing queue size
    if (outQueue.size() == 0) return null;
    return (OutgoingObject) outQueue.remove(0);
  }

  public void addOutgoingObject(Response response) { // todo lock incoming queue by outgoing queue size
    if (response == null)
      return;
    if (response.getMessages() == null)
      return;
    switch (response.getResponseType()) {
      case Response.TYPE_ARRAY:
        for (int i = 0; i < response.getMessages().length; i++) {
          addOutgoingObject(new OutgoingObject(response.getMessages()[i]));
        }
        break;
    }
  }

  public boolean registerMessageStatusListener(MessageStatusListener listener) {
    return messageStatusChangedNotifier.registerMessageStatusListener(listener);
  }

  public synchronized void addOutgoingObject(OutgoingObject obj) { // todo see upper
    putOutgoingObject(obj);
  }

  private synchronized void putOutgoingObject(OutgoingObject obj) {
    if (obj.getRetryPeriod() < retryPeriod) {
      obj.setRetryPeriod(retryPeriod);
    }
    outQueue.add(obj);
    if (Logger.isInfoEnabled())
      Logger.info("OUT " + (obj.getSendTime() > -1 ? "readded" : "added") + ", queue size=" + outQueue.size() + "; sent size = " + sendTasks.size() + "; retry size = " + retryTasks.size());
    synchronized (sendNotifyMonitor) {
      sendNotifyMonitor.notifyAll();
    }
  }

  public synchronized void messageSubmitted(OutgoingObject obj) {
    obj.send();
    sendTasks.add(obj);
  }

  public synchronized void restoreOutgoingObject(int connectionId, int sequenceId) {
    int idx = sendTasks.indexOf(new OutgoingObjectIndex(connectionId, sequenceId));
    if (idx > -1) {
      OutgoingObject obj = (OutgoingObject) sendTasks.remove(idx);
      outQueue.add(0, obj);
      if (Logger.isDebugEnabled())
        Logger.debug("OUT inserted in queue head by throttled error, queue size=" + outQueue.size() + "; sent size = " + sendTasks.size() + "; retry size = " + retryTasks.size());
      synchronized (sendNotifyMonitor) {
        sendNotifyMonitor.notifyAll();
      }
    } else {
      if (Logger.isDebugEnabled())
        Logger.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] not found in sendTasks container");
      return;
    }
  }

  public synchronized void removeOutgoingObject(int connectionId, int sequenceId, int status) {
    int idx = sendTasks.indexOf(new OutgoingObjectIndex(connectionId, sequenceId));
    String ab_addr = "";

    if (idx > -1) {
      OutgoingObject obj = (OutgoingObject) sendTasks.remove(idx);
      if (obj.getOutgoingMessage() != null) {
        ab_addr = obj.getOutgoingMessage().getDestinationAddress();
      }
      messageStatusChangedNotifier.notifyListener(obj.getMessageStatusListenerName(), obj.getId(), status);
    } else {
      if (Logger.isDebugEnabled())
        Logger.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] not found in sendTasks container");
      return;
    }
    if (status == PDU.STATUS_CLASS_NO_ERROR) {
      if (Logger.isDebugEnabled())
        Logger.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + ";AB=" + ab_addr + "] finalized, submit successful: " + status);
    } else if (status == PDU.STATUS_CLASS_PERM_ERROR) {
      if (Logger.isDebugEnabled())
        Logger.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + ";AB=" + ab_addr + "] finalized, permanent error received: " + status);
    } else {
      if (Logger.isDebugEnabled())
        Logger.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + ";AB=" + ab_addr + "] finalized, unexpected status received: " + status);
    }
  }

  public void updateOutgoingObject(int connectionId, int sequenceId) {
    updateOutgoingObject(connectionId, sequenceId, -1);
  }

  public synchronized void updateOutgoingObject(int connectionId, int sequenceId, int status) {
    int idx = sendTasks.indexOf(new OutgoingObjectIndex(connectionId, sequenceId));
    if (idx > -1) {
      OutgoingObject obj = (OutgoingObject) sendTasks.remove(idx);
      String ab_addr = "";
      if (obj.getOutgoingMessage() != null)
        ab_addr = obj.getOutgoingMessage().getDestinationAddress();
      if (obj.getRetries() < maxRetry) {
        if (obj.getOutgoingMessage().hasUssdServiceOp()) {
          if (Logger.isDebugEnabled())
            Logger.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + ";AB=" + ab_addr + "] finalized, submit failed but USSD could not be rescheduled: " + status);
          messageStatusChangedNotifier.notifyListener(obj.getMessageStatusListenerName(), obj.getId(), status);
        } else {
          retryTasks.add(new RetryTask(obj));
          if (Logger.isDebugEnabled())
            Logger.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + ";AB=" + ab_addr + "] rescheduled, attempt #" + obj.getRetries() + " submit failed: " + status);
        }
      } else {
        if (Logger.isDebugEnabled())
          Logger.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + ";AB=" + ab_addr + "] finalized, max attempts value was reached: " + status);
        messageStatusChangedNotifier.notifyListener(obj.getMessageStatusListenerName(), obj.getId(), status);
      }
    } else {
      if (Logger.isDebugEnabled())
        Logger.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] not found in sendTasks container: " + status);
    }
  }

  private void clearQueue() {
    OutgoingObject obj;
    while (sendTasks.size() > 0) {
      obj = (OutgoingObject) sendTasks.get(0);
      if (obj.getSendTime() + submitTimeout < System.currentTimeMillis()) {
        sendTasks.remove(0);
        if (obj.getRetries() < maxRetry) {
          if (obj.getOutgoingMessage().hasUssdServiceOp()) {
            if (Logger.isDebugEnabled())
              Logger.debug("OUT [CID=" + obj.getOutgoingMessage().getConnectionId() + ";SN=" + obj.getOutgoingMessage().getSequenceNumber() + "] cleared, submit timout reached but USSD could not be rescheduled");
          } else {
            retryTasks.add(new RetryTask(obj));
            if (Logger.isDebugEnabled())
              Logger.debug("OUT [CID=" + obj.getOutgoingMessage().getConnectionId() + ";SN=" + obj.getOutgoingMessage().getSequenceNumber() + "] rescheduled, attempt #" + obj.getRetries() + " submit timeout reached");
          }
        } else {
          if (Logger.isDebugEnabled())
            Logger.debug("OUT [CID=" + obj.getOutgoingMessage().getConnectionId() + ";SN=" + obj.getOutgoingMessage().getSequenceNumber() + "] finalized, max attempts value was reached");
        }
      } else
        break;
    }
  }

  public synchronized void reschedule() {
    clearQueue();
    RetryTask task;
    while (!retryTasks.isEmpty()) {
      task = (RetryTask) retryTasks.first();
      if (task.getRetryTime() < System.currentTimeMillis()) {
        retryTasks.remove(task);
        putOutgoingObject(task.getOutgoingObject());
        synchronized (sendNotifyMonitor) {
          sendNotifyMonitor.notifyAll();
        }
      } else
        break;
    }
    if (Logger.isDebugEnabled())
      Logger.debug("OUT QUEUE STATUS: out size=" + outQueue.size() + "; sent size = " + sendTasks.size() + "; retry size = " + retryTasks.size());
  }

  public void setSendNotifyMonitor(Object sendNotifyMonitor) {
    this.sendNotifyMonitor = sendNotifyMonitor;
  }

  public MessageSender getMessageSender() {
    return messageSender;
  }

  public void setMessageSender(MessageSender messageSender) {
    this.messageSender = messageSender;
  }
}
