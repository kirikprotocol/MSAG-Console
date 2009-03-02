package ru.sibinco.smsx.engine.service.calendar;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarCheckMessageStatusCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarHandleReceiptCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarDataSource;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarMessage;
import ru.sibinco.smsx.utils.DataSourceException;

import java.sql.Timestamp;
import java.util.Calendar;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 28.06.2007
 */

class CalendarProcessor implements CalendarSendMessageCmd.Receiver, CalendarCheckMessageStatusCmd.Receiver, CalendarHandleReceiptCmd.Receiver {
  private static final Category log = Category.getInstance("CALENDAR");

  private static final Pattern ALLOWED_DEST_ADDR = Pattern.compile("\\+\\d{11}|0012");

  // Properties
  private final long maxDate;

  private final MessagesQueue messagesQueue;
  private final CalendarDataSource ds;
  private final int serviceId;

  CalendarProcessor(MessagesQueue messagesQueue, CalendarDataSource ds, int maxYear, int serviceId) {
    this.messagesQueue = messagesQueue;
    this.ds = ds;
    this.serviceId = serviceId;

    final Calendar cal = Calendar.getInstance();
    cal.setTimeInMillis(0);
    cal.set(Calendar.YEAR, maxYear + 1);
    maxDate = cal.getTimeInMillis();
  }

  public long execute(CalendarSendMessageCmd cmd) throws CommandExecutionException {
    if (log.isInfoEnabled())
        log.info("Req: srcaddr=" + cmd.getSourceAddress() + "; dstaddr=" + cmd.getDestinationAddress() + "; senddate=" + cmd.getSendDate());

    // Check send date
    if (cmd.getSendDate() == null || cmd.getSendDate().getTime() > maxDate || cmd.getSendDate().getTime() < System.currentTimeMillis()) {
      if (log.isInfoEnabled())
        log.info("Send date is empty, in the past or after max year");
      throw new CommandExecutionException("Invalid send date", CalendarSendMessageCmd.ERR_WRONG_SEND_DATE);
    }

    if (!ALLOWED_DEST_ADDR.matcher(cmd.getDestinationAddress()).matches()) {
      if (log.isInfoEnabled())
        log.info("Destination address is not allowed");
      throw new CommandExecutionException("Invalid destination address", CalendarSendMessageCmd.ERR_WRONG_DESTINATION_ADDRESS);
    }

    try {
      // Create message
      final CalendarMessage calendarMessage = new CalendarMessage();
      calendarMessage.setSourceAddress(cmd.getSourceAddress());
      calendarMessage.setDestinationAddress(cmd.getDestinationAddress());
      calendarMessage.setSendDate(new Timestamp(cmd.getSendDate().getTime()));
      calendarMessage.setDestAddressSubunit(cmd.getDestAddressSubunit());
      calendarMessage.setMessage(cmd.getMessage());
      calendarMessage.setSaveDeliveryStatus(cmd.isStoreDeliveryStatus());
      calendarMessage.setConnectionName(cmd.getSourceId() == Command.SOURCE_SMPP ? "smsx" : "websms");
      calendarMessage.setMscAddress(cmd.getMscAddress());
      calendarMessage.setAppendAdvertising(cmd.isAppendAdvertising());

      // Save message
      ds.saveCalendarMessage(calendarMessage);
      if (log.isInfoEnabled())
        log.info("Msg was stored in DB");

      if (messagesQueue.add(calendarMessage) && log.isInfoEnabled())
        log.info("Msg was puted into msgs list");

      return calendarMessage.getId();

    } catch (DataSourceException e) {
      log.error("Req err for " + cmd.getSourceAddress(), e);
      throw new CommandExecutionException("Error: " + e.getMessage(), CalendarSendMessageCmd.ERR_SYS_ERROR);
    }
  }

  public int execute(CalendarCheckMessageStatusCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("Get msg status: id=" + cmd.getMsgId());

      final CalendarMessage msg = ds.loadCalendarMessageById(cmd.getMsgId());

      int status;
      if (msg != null && msg.isSaveDeliveryStatus()) {
        status = msg.getStatus();
      } else {
        if (log.isInfoEnabled())
          log.info("Msg with id=" + cmd.getMsgId() + " not found");
        status = CalendarCheckMessageStatusCmd.MESSAGE_STATUS_UNKNOWN;
      }

      return status;

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException("Error: " + e.getMessage(), CalendarCheckMessageStatusCmd.ERR_SYS_ERROR);
    }
  }

  public boolean execute(CalendarHandleReceiptCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("Handle rcpt: umr=" + cmd.getUmr() + "; dlvr=" + cmd.isDelivered());

      if (cmd.getUmr() % 10 == serviceId) {
        ds.updateMessageStatus(cmd.getUmr() / 10, cmd.isDelivered() ? CalendarMessage.STATUS_DELIVERED : CalendarMessage.STATUS_DELIVERY_FAILED);
        return true;
      }

      return false;

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException("Error: " + e.getMessage(), CalendarCheckMessageStatusCmd.ERR_SYS_ERROR);
    }
  }
}
