package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.utils.ds.DataSourceException;
import com.eyeline.sme.utils.worker.IterativeWorker;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarDataSource;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarMessage;
import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;

import java.util.Date;
import java.util.Iterator;

/**
 * User: artem
 * Date: 11.07.2007
 */

class CalendarEngine extends IterativeWorker {

  private static final Category log = Category.getInstance("CALENDAR");

  private final MessagesQueue messagesQueue;

  private final Date nextReloadTime = new Date();
  private final SMPPOutgoingQueue outQueue;
  private final long workingInterval;
  private final CalendarDataSource ds;

  CalendarEngine(SMPPOutgoingQueue outQueue, MessagesQueue messagesQueue, CalendarDataSource ds, long workingInterval) {
    super(log);

    this.outQueue = outQueue;
    this.workingInterval = workingInterval;
    this.ds = ds;
    this.messagesQueue = messagesQueue;
  }

  protected void stopCurrentWork() {
    messagesQueue.notifyWaiters();
  }

  public void iterativeWork() {
    nextReloadTime.setTime(System.currentTimeMillis() + workingInterval);

    if (log.isInfoEnabled())
      log.info("New period: enddate=" + nextReloadTime);

    messagesQueue.setMaxDate(nextReloadTime);
    loadList();

    CalendarMessage msg;
    while (System.currentTimeMillis() < nextReloadTime.getTime() && isStarted()) {
      if ((msg = messagesQueue.getNext()) == null)
        continue;

      sendMessage(msg);

      // Change message status
      msg.setStatus(CalendarMessage.STATUS_PROCESSED);
      try {
        ds.saveCalendarMessage(msg);
      } catch (Throwable e) {
        log.error(e, e);
        log.error("Can't remove msg: ", e);
      }
    }
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

      final CalendarTransportObject outObj = new CalendarTransportObject(message);
      outObj.setOutgoingMessage(msg);
      outQueue.addOutgoingObject(outObj);

    } catch (Throwable e) {
      log.error("Can't send msg: ",e);
    }
  }

  private void loadList() {
    try {
      for (Iterator iter = ds.loadCalendarMessages(nextReloadTime, 10000).iterator(); iter.hasNext();)
        messagesQueue.add((CalendarMessage)iter.next());

    } catch (Throwable e) {
      log.error("Can't load msgs list: ", e);
    }
  }


  private class CalendarTransportObject extends SMPPTransportObject {
    private final CalendarMessage msg;

    CalendarTransportObject(CalendarMessage msg) {
      this.msg = msg;
    }

    public void handleResponse(PDU pdu) {
      try {
        if (msg.isSaveDeliveryStatus()) {
          msg.setStatus(pdu.getStatusClass() == PDU.STATUS_CLASS_NO_ERROR ? CalendarMessage.STATUS_DELIVERED : CalendarMessage.STATUS_DELIVERY_FAILED);
          msg.setSmppStatus(pdu.getStatus());
          ds.saveCalendarMessage(msg);
        }
      } catch (DataSourceException e) {
        log.error("Can't save delivery status!");
      }
    }

    public void handleSendError() {
      try {
        msg.setStatus(CalendarMessage.STATUS_DELIVERY_FAILED);
        ds.saveCalendarMessage(msg);
      } catch (DataSourceException e) {
        log.error("Can't save calendar message", e);
      }
    }
  }
}
