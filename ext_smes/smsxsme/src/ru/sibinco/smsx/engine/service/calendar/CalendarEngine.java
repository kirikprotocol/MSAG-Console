package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.utils.worker.IterativeWorker;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.utils.ThreadFactoryWithCounter;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SubmitResponse;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarDataSource;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarMessage;
import ru.sibinco.smsx.utils.DataSourceException;

import java.util.Date;
import java.util.Iterator;
import java.util.concurrent.*;

/**
 * User: artem
 * Date: 11.07.2007
 */

class CalendarEngine extends IterativeWorker {

  private static final Category log = Category.getInstance("CALENDAR");

  private final MessagesQueue messagesQueue;

  private final Date nextReloadTime = new Date();
  private final OutgoingQueue outQueue;
  private final long workingInterval;
  private final CalendarDataSource ds;
  private final ThreadPoolExecutor executor;

  private volatile int rejectedTasks;  

  CalendarEngine(OutgoingQueue outQueue, MessagesQueue messagesQueue, CalendarDataSource ds, long workingInterval) {
    super(log);

    this.outQueue = outQueue;
    this.workingInterval = workingInterval;
    this.ds = ds;
    this.messagesQueue = messagesQueue;
    this.executor = new ThreadPoolExecutor(3, 10, 60L, TimeUnit.SECONDS, new ArrayBlockingQueue(100), new ThreadFactoryWithCounter("CalEngine-Executor-"));
  }

  protected void stopCurrentWork() {
    messagesQueue.notifyWaiters();
    executor.shutdown();
  }

  public void iterativeWork() {
    nextReloadTime.setTime(System.currentTimeMillis() + workingInterval);

    messagesQueue.setMaxDate(nextReloadTime);
    loadList();

    CalendarMessage msg;
    while ((messagesQueue.size() > 0 || System.currentTimeMillis() < nextReloadTime.getTime()) && isStarted()) {
      if ((msg = messagesQueue.getNext()) == null)
        continue;

      sendMessage(msg);

      // Change message status
      msg.setStatus(CalendarMessage.STATUS_PROCESSED);
      try {
        ds.updateMessageStatus(msg);
      } catch (Throwable e) {
        log.error(e, e);
        log.error("Can't remove msg: ", e);
      }
    }
  }

  public Date getEndDate() {
    return nextReloadTime;
  }

  public Date getCurrentDate() {
    return new Date();
  }

  public int getQueueSize() {
    return messagesQueue.size();
  }

  public int getExecutorActiveCount() {
    return executor.getActiveCount();
  }

  public int getExecutorPoolSize() {
    return executor.getPoolSize();
  }

  public int getExecutorMaxPoolSize() {
    return executor.getMaximumPoolSize();
  }

  public void setExecutorMaxPoolSize(int size) {
    executor.setMaximumPoolSize(size);
  }

  public int getExecutorRejectedTasks() {
    return rejectedTasks;
  }

  private void sendMessage(final CalendarMessage message) {
    try {
      if (log.isInfoEnabled())
        log.info("Send msg: srcaddr=" + message.getSourceAddress() + "; dstaddr=" + message.getDestinationAddress() + "; senddate=" + message.getSendDate());

      final Message msg = new Message();
      msg.setSourceAddress(message.getSourceAddress());
      msg.setDestinationAddress(message.getDestinationAddress());
      msg.setMessageString(message.getMessage());
      msg.setDestAddrSubunit(message.getDestAddressSubunit());
      msg.setConnectionName(message.getConnectionName());
      if (message.isSaveDeliveryStatus())
        msg.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);

      final CalendarTransportObject outObj = new CalendarTransportObject(message);
      outObj.setMessage(msg);
      outQueue.offer(outObj);

    } catch (Throwable e) {
      log.error("Can't send msg: ",e);
    }
  }

  private void loadList() {
    try {
      for (CalendarMessage o : ds.loadCalendarMessages(nextReloadTime, 10000))
        messagesQueue.add(o);

    } catch (Throwable e) {
      log.error("Can't load msgs list: ", e);
    }
  }


  private class CalendarTransportObject extends OutgoingObject {
    private final CalendarMessage msg;

    CalendarTransportObject(CalendarMessage msg) {
      this.msg = msg;
    }

    @Override
    public void handleResponse(final PDU pdu) {
      if (msg.isSaveDeliveryStatus()) {
        if (pdu.getStatusClass() == PDU.STATUS_CLASS_PERM_ERROR) {
          try {
            executor.execute(new UpdateMessageStatusTask());
          } catch (Throwable e) {
            log.error("Can't execute UpdateMessageStatusTask", e);
            rejectedTasks++;
          }
        } else if (pdu.getStatusClass() == PDU.STATUS_CLASS_NO_ERROR) {
          try {
            executor.execute(new UpdateSMPPIdTask(Long.parseLong(((SubmitResponse)pdu).getMessageId())));
          } catch (Throwable e) {
            log.error("Can't execute UpdateSMPPIdTask", e);
            rejectedTasks++;
          }
        }
      }
    }

    @Override
    public void handleSendError() {
      if (msg.isSaveDeliveryStatus()) {
        try {
          executor.execute(new UpdateMessageStatusTask());
        } catch (Throwable e) {
          log.error("Can't execute UpdateMessageStatusTask", e);
          rejectedTasks++;
        }
      }
    }

    private class UpdateMessageStatusTask implements Runnable {
      public void run() {
        try {
          msg.setStatus(CalendarMessage.STATUS_DELIVERY_FAILED);
          ds.updateMessageStatus(msg);
        } catch (Throwable e) {
          log.error("Update msg status err:",e);
        }
      }
    }

    private class UpdateSMPPIdTask implements Runnable {
      private final long messageId;

      private UpdateSMPPIdTask(long messageId) {
        this.messageId = messageId;
      }

      public void run() {
        msg.setSmppId(messageId);
        try {
          ds.updateMessageSmppId(msg);
        } catch (Throwable e) {
          log.error("Can't update smpp id: ", e);
        }
      }
    }
  }
}
