package ru.sibinco.smsx.network;

import ru.aurorisoft.smpp.PDU;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.utils.Utils;

import java.util.*;

public class OutgoingQueue {
  /**
   * Logger.
   */
  private final static org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(OutgoingQueue.class);
  /**
   * List of tasks with start time, for tracking submit timeouts.
   */
  private final List sendTasks;
  /**
   * List of tasks with start time, for messages that should be retried.
   */
  private final SortedSet retryTasks;
  /**
   * After this period the submit will be rescheduled.
   */
  private final long submitTimeout;
  /**
   * After this retry message will be deleted.
   */
  private final long maxRetry;
  /**
   * After this period the submit will be retryed.
   */
  private final long retryPeriod;
  /**
   * Outgoing queue.
   */
  private final List outQueue;
  /**
   * Incoming queue (to initiate network session after submit response)
   */
  private boolean outQueueEmpty = true;

  public OutgoingQueue(final Properties config) {

    submitTimeout = Utils.loadLong(config, "outgoing.queue.submit.timeout");

    retryPeriod = Utils.loadLong(config, "outgoing.queue.retry.period");
    if (retryPeriod < submitTimeout)
      throw new InitializationException("Outgoing queue retryPeriod < submitTimeout");

    maxRetry = Utils.loadLong(config, "outgoing.queue.max.retries");
    if (maxRetry < 1)
      throw new InitializationException("Outgoing queue max retries < 1");

    outQueue = new LinkedList();
    sendTasks = new LinkedList();
    retryTasks = new TreeSet(new RetryTask());

    Log.info("OutgoingQueue created.");
  }

  public synchronized OutgoingObject getOutgoingObject() {
    if (outQueueEmpty) {
      try {
        wait();
      } catch (InterruptedException e) {
        Log.error("OutgoingQueue: interrupted: " + e);
      }
    }

    synchronized(outQueue) {
      final OutgoingObject outObj = (OutgoingObject) outQueue.remove(0);
      outQueueEmpty = outQueue.isEmpty();
      return outObj;
    }
  }

  public synchronized void addOutgoingObject(OutgoingObject obj) {
    if (obj.getRetryPeriod() < retryPeriod) {
      obj.setRetryPeriod(retryPeriod);
    }
    synchronized(outQueue) {
      outQueue.add(obj);
      Log.debug("OUT " + (obj.getSendTime() > -1 ? "readded" : "added") + ", queue size=" + outQueue.size() + "; sent size = " + sendTasks.size() + "; retry size = " + retryTasks.size());
      outQueueEmpty = false;
      notify();
    }

  }

  public synchronized void messageSubmitted(OutgoingObject obj) {
    obj.send();
    sendTasks.add(obj);
  }

  public synchronized void removeOutgoingObject(int connectionId, int sequenceId, int status) {
    int idx = sendTasks.indexOf(new OutgoingObjectIndex(connectionId, sequenceId));
    OutgoingObject obj;
    if (idx > -1) {
      obj = (OutgoingObject) sendTasks.remove(idx);
    } else {
      Log.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] not found in sendTasks container");
      return;
    }
    if (status == PDU.STATUS_CLASS_NO_ERROR) {
      Log.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] finalized, submit successful");

      obj.changeStatus(OutgoingObject.DELIVER_OK);

    } else if (status == PDU.STATUS_CLASS_PERM_ERROR) {
      Log.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] finalized, permanent error received");
      obj.changeStatus(OutgoingObject.DELIVER_PERM_ERROR);
    } else {
      Log.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] finalized, unexpected status received: " + status);
      obj.changeStatus(OutgoingObject.DELIVER_UNKNOWN_ERROR);
    }
  }

  public synchronized void updateOutgoingObject(int connectionId, int sequenceId) {
    int idx = sendTasks.indexOf(new OutgoingObjectIndex(connectionId, sequenceId));
    if (idx > -1) {
      OutgoingObject obj = (OutgoingObject) sendTasks.remove(idx);
      obj.changeStatus(OutgoingObject.DELIVER_TMP_ERROR);
      if (obj.getRetries() < maxRetry) {
        if (obj.getOutgoingMessage().hasUssdServiceOp()) {
          Log.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] finalized, submit failed but USSD could not be rescheduled");
        } else {
          retryTasks.add(new RetryTask(obj));
          Log.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] rescheduled, submit failed");
        }
      } else {
        Log.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] finalized, max attempts value was reached");
      }
    } else {
      Log.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] not found in sendTasks container");
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
            Log.debug("OUT [CID=" + obj.getOutgoingMessage().getConnectionId() + ";SN=" + obj.getOutgoingMessage().getSequenceNumber() + "] cleared, submit timout reached but USSD could not be rescheduled");
          } else {
            retryTasks.add(new RetryTask(obj));
            Log.debug("OUT [CID=" + obj.getOutgoingMessage().getConnectionId() + ";SN=" + obj.getOutgoingMessage().getSequenceNumber() + "] rescheduled, submit timeout reached");
          }
        } else {
          Log.debug("OUT [CID=" + obj.getOutgoingMessage().getConnectionId() + ";SN=" + obj.getOutgoingMessage().getSequenceNumber() + "] finalized, max attempts value was reached");
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
        addOutgoingObject(task.getOutgoingObject());
      } else
        break;
    }
  }

  public synchronized void restoreOutgoingObject(int connectionId, int sequenceId) {
    int idx = sendTasks.indexOf(new OutgoingObjectIndex(connectionId, sequenceId));
    if (idx > -1) {
      OutgoingObject obj = (OutgoingObject)sendTasks.remove(idx);
      obj.changeStatus(OutgoingObject.DELIVER_THROTTLED_ERROR);
      synchronized(outQueue) {
        outQueue.add(0, obj);
        Log.debug("OUT inserted in queue head by throttled error, queue size=" + outQueue.size() + "; sent size = " + sendTasks.size() + "; retry size = " + retryTasks.size());
        outQueueEmpty = false;
      }
    } else {
      Log.debug("OUT [CID=" + connectionId + ";SN=" + sequenceId + "] not found in sendTasks container");
    }
  }

}
