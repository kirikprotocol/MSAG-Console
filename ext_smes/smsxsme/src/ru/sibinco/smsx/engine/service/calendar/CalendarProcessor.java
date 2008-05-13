package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.utils.ds.DataSourceException;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarCheckMessageStatusCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarHandleReceiptCmd;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarDataSource;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarMessage;
import ru.sibinco.smsx.engine.service.Command;

import java.sql.Timestamp;
import java.util.Calendar;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 28.06.2007
 */

class CalendarProcessor implements CalendarSendMessageCmd.Receiver, CalendarCheckMessageStatusCmd.Receiver, CalendarHandleReceiptCmd.Receiver {
  private static final Category log = Category.getInstance("CALENDAR");

  private static final Pattern ALLOWED_DEST_ADDR = Pattern.compile("\\+\\d{11}");

  // Properties
  private final long maxDate;

  private final MessagesQueue messagesQueue;
  private final CalendarDataSource ds;

  CalendarProcessor(MessagesQueue messagesQueue, CalendarDataSource ds, int maxYear) {
    this.messagesQueue = messagesQueue;
    this.ds = ds;

    final Calendar cal = Calendar.getInstance();
    cal.setTimeInMillis(0);
    cal.set(Calendar.YEAR, maxYear + 1);
    maxDate = cal.getTimeInMillis();
  }

  public void execute(CalendarSendMessageCmd cmd) {
    try {
      if (log.isInfoEnabled())
        log.info("Req: srcaddr=" + cmd.getSourceAddress() + "; dstaddr=" + cmd.getDestinationAddress() + "; senddate=" + cmd.getSendDate());

      // Check send date
      if (cmd.getSendDate() == null || cmd.getSendDate().getTime() > maxDate || cmd.getSendDate().getTime() < System.currentTimeMillis()) {
        if (log.isInfoEnabled())
          log.info("Send date is empty, in the past or after max year");
        cmd.update(CalendarSendMessageCmd.STATUS_WRONG_SEND_DATE);
        return;

      } else if (!ALLOWED_DEST_ADDR.matcher(cmd.getDestinationAddress()).matches()) {
        if (log.isInfoEnabled())
          log.info("Destination address is not allowed");
        cmd.update(CalendarSendMessageCmd.STATUS_WRONG_DESTINATION_ADDRESS);
        return;

      } else {

        // Create message
        final CalendarMessage calendarMessage = new CalendarMessage();
        calendarMessage.setSourceAddress(cmd.getSourceAddress());
        calendarMessage.setDestinationAddress(cmd.getDestinationAddress());
        calendarMessage.setSendDate(new Timestamp(cmd.getSendDate().getTime()));
        calendarMessage.setDestAddressSubunit(cmd.getDestAddressSubunit());
        calendarMessage.setMessage(cmd.getMessage());
        calendarMessage.setSaveDeliveryStatus(cmd.isStoreDeliveryStatus());
        calendarMessage.setConnectionName(cmd.getSourceId() == Command.SOURCE_SMPP ? "smsx" : "websms");

        // Save message
        ds.saveCalendarMessage(calendarMessage);
        if (log.isInfoEnabled())
          log.info("Msg was stored in DB");

        if (messagesQueue.add(calendarMessage) && log.isInfoEnabled())
          log.info("Msg was puted into msgs list");

        cmd.setMsgId(calendarMessage.getId());

        cmd.update(CalendarSendMessageCmd.STATUS_SUCCESS);
      }

    } catch (Throwable e) {
      log.error("Req err for " + cmd.getSourceAddress(), e);
      cmd.update(CalendarSendMessageCmd.STATUS_SYSTEM_ERROR);
    }
  }

  public void execute(CalendarCheckMessageStatusCmd cmd) {
    try {
      if (log.isInfoEnabled())
        log.info("Get msg status: id=" + cmd.getMsgId());

      final CalendarMessage msg = ds.loadCalendarMessageById(cmd.getMsgId());

      if (msg != null && msg.isSaveDeliveryStatus()) {
        cmd.setMessageStatus(msg.getStatus());
        cmd.setSmppStatus(msg.getSmppStatus());
        cmd.setSaveDeliveryStatus(msg.isSaveDeliveryStatus());

      } else {
        if (log.isInfoEnabled())
          log.info("Msg with id=" + cmd.getMsgId() + " not found");
        cmd.setMessageStatus(CalendarCheckMessageStatusCmd.MESSAGE_STATUS_UNKNOWN);
      }

      cmd.update(CalendarCheckMessageStatusCmd.STATUS_SUCCESS);

    } catch (DataSourceException e) {
      log.error(e,e);
      cmd.update(CalendarCheckMessageStatusCmd.STATUS_SYSTEM_ERROR);
    }
  }

  public boolean execute(CalendarHandleReceiptCmd cmd) {
    try {
      if (log.isInfoEnabled())
        log.info("Handle rcpt: id=" + cmd.getSmppMessageId() + "; dlvr=" + cmd.isDelivered());

      int result = ds.updateMessageStatus(cmd.getSmppMessageId(), cmd.isDelivered() ? CalendarMessage.STATUS_DELIVERED : CalendarMessage.STATUS_DELIVERY_FAILED);
      return result > 0;

    } catch (DataSourceException e) {
      log.error(e,e);
      cmd.update(CalendarHandleReceiptCmd.STATUS_SYSTEM_ERROR);
    }

    return true;
  }
}
