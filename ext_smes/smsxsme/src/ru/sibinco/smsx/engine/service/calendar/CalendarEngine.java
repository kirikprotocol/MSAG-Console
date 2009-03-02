package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.utils.worker.IterativeWorker;
import com.eyeline.utils.ThreadFactoryWithCounter;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarDataSource;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarMessage;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.network.advertising.AdvertisingClientException;

import java.util.Date;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

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
  private final AdvertisingClient advClient;
  private String advDelim;
  private int advSize;
  private String advService;
  private final int serviceId;

  private volatile int rejectedTasks;

  CalendarEngine(OutgoingQueue outQueue, MessagesQueue messagesQueue, CalendarDataSource ds, AdvertisingClient advClient, long workingInterval, int serviceId) {
    super(log);

    this.outQueue = outQueue;
    this.workingInterval = workingInterval;
    this.ds = ds;
    this.advClient = advClient;
    this.messagesQueue = messagesQueue;
    this.serviceId = serviceId;
    this.executor = new ThreadPoolExecutor(1, 10, 60L, TimeUnit.SECONDS, new ArrayBlockingQueue<Runnable>(100), new ThreadFactoryWithCounter("CalEngine-Executor-"));
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
    }
  }

  public String getAdvDelim() {
    return advDelim;
  }

  public void setAdvDelim(String advDelim) {
    this.advDelim = advDelim;
  }

  public int getAdvSize() {
    return advSize;
  }

  public void setAdvSize(int advSize) {
    this.advSize = advSize;
  }

  public String getAdvService() {
    return advService;
  }

  public void setAdvService(String advService) {
    this.advService = advService;
  }

  public Date getEndDate() {
    return nextReloadTime;
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
      msg.setDestAddrSubunit(message.getDestAddressSubunit());
      msg.setConnectionName(message.getConnectionName());
      msg.setMscAddress(message.getMscAddress());

      String messageString = message.getMessage();
      if (message.isAppendAdvertising()) {
        final String banner = (advSize > 0) ? getBannerForAbonent(message.getDestinationAddress(), advSize - messageString.length() - advDelim.length()) : getBannerForAbonent(message.getDestinationAddress());
        if (log.isInfoEnabled())
          log.info("Append banner: " + banner);
        if (banner != null)
          messageString += advDelim + banner;
      }
      msg.setMessageString(messageString);

      OutgoingObject outObj = new CalendarOutgoingObject(message);
      if (message.isSaveDeliveryStatus()) {
        msg.setUserMessageReference(message.getId() * 10 + serviceId);
        msg.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);
      }

      outObj.setMessage(msg);
      outQueue.offer(outObj);

    } catch (Throwable e) {
      log.error("Can't send msg: ",e);
    }
  }

  private String getBannerForAbonent(final String abonentAddress) {
    try {
      return advClient.getBanner(advService, abonentAddress);
    } catch (AdvertisingClientException e) {
      log.error("Can't get banner for abonent " + abonentAddress, e);
      return null;
    }
  }

  private String getBannerForAbonent(final String abonentAddress, int maxBannerLength) {
    try {
      return advClient.getBanner(advService, abonentAddress, maxBannerLength);
    } catch (AdvertisingClientException e) {
      log.error("Can't get banner for abonent " + abonentAddress, e);
      return null;
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

  private class CalendarOutgoingObject extends OutgoingObject {
    private final CalendarMessage msg;

    private CalendarOutgoingObject(CalendarMessage msg) {
      this.msg = msg;
    }

    public void handleResponse(PDU pdu) {
      if (pdu.getStatusClass() == PDU.STATUS_CLASS_PERM_ERROR) {
        updateStatus(CalendarMessage.STATUS_DELIVERY_FAILED);
      } else if (pdu.getStatusClass() == PDU.STATUS_CLASS_NO_ERROR) {
        updateStatus(CalendarMessage.STATUS_PROCESSED);
      }
    }

    public void handleSendError() {
      updateStatus(CalendarMessage.STATUS_DELIVERY_FAILED);
    }

    private void updateStatus(int status) {
      try {
        executor.execute(new UpdateMessageStatusTask(status));
      } catch (Throwable e) {
        log.error("Can't execute UpdateMessageStatusTask", e);
        rejectedTasks++;
      }
    }

    private class UpdateMessageStatusTask implements Runnable {
      private final int status;

      private UpdateMessageStatusTask(int status) {
        this.status = status;
      }

      public void run() {
        try {
          msg.setStatus(status);
          ds.updateMessageStatus(msg);
        } catch (Throwable e) {
          log.error("Update msg status err:",e);
        }
      }
    }
  }

}
