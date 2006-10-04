package ru.sibinco.calendarsme.engine.calendar;

import ru.aurorisoft.smpp.Message;
import ru.sibinco.calendarsme.InitializationException;
import ru.sibinco.calendarsme.network.OutgoingObject;
import ru.sibinco.calendarsme.network.OutgoingQueue;
import ru.sibinco.calendarsme.utils.ConnectionPool;
import ru.sibinco.calendarsme.utils.Service;
import ru.sibinco.calendarsme.utils.Utils;

import java.sql.*;
import java.util.Date;
import java.util.Properties;

/**
 * User: artem
 * Date: Jul 31, 2006
 */

final class CalendarEngine extends Service {
  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(CalendarEngine.class);

  private final  CalendarMessagesList messagesList;

  private final Object newMessageNotifier = new Object();

  private final String loadListSQL;
  private final String removeMessageSQL;

  private final long workingInterval;
  private final Date nextReloadTime = new Date();
  private final OutgoingQueue outQueue;

  public CalendarEngine(final Properties config, final OutgoingQueue outQueue, final CalendarMessagesList messagesList) {
    super(Log);

    if (config == null)
      throw new InitializationException("CalendarEngine: config is not specified");
    if (outQueue == null)
      throw new InitializationException("CalendarEngine: outgoing queue is not specified");

    this.outQueue = outQueue;

    this.messagesList = messagesList;
    messagesList.addNewMessageNotifier(newMessageNotifier);

    this.loadListSQL = Utils.loadString(config, "calendar.engine.load.list.sql");
    this.removeMessageSQL = Utils.loadString(config, "calendar.engine.remove.message.sql");
    this.workingInterval = Utils.loadLong(config, "calendar.engine.working.interval");

  }

  protected void doBeforeStart() {
    loadList();
  }

  public synchronized void iterativeWork() {
    Log.info("=====================================================================================");
    nextReloadTime.setTime(System.currentTimeMillis() + workingInterval);
    Log.info("Start new time period. End time= " + nextReloadTime.toString());

    boolean hasDBMessages = true;

    while (System.currentTimeMillis() < nextReloadTime.getTime()) {
      Log.info("Process messages list.");

      if (hasDBMessages && messagesList.isEmpty())
        hasDBMessages = loadList();

      if (messagesList.isEmpty()) {
        Log.info("Messages list is empty. Waiting for the end of period or new messages...");
        try {
          synchronized(newMessageNotifier) {
            newMessageNotifier.wait(nextReloadTime.getTime() - System.currentTimeMillis());
          }
        } catch (InterruptedException e) {
          Log.error("Interrupted:", e);
        } catch (IllegalArgumentException e) {
          Log.error("Interrupted:", e);
        }
      }

      while (!messagesList.isEmpty()) {
        CalendarMessage currentMessage = messagesList.getFirst();
        Log.info("Processing new message from list");
        if (currentMessage != null) {
          // Waiting for current message send time
          final long waitTime = currentMessage.getSendDate().getTime() - System.currentTimeMillis();
          if (waitTime > 0) {
            Log.info("Next message send time = " + currentMessage.getSendDate() + "; waiting...");
            try {
              synchronized(newMessageNotifier) {
                newMessageNotifier.wait(waitTime);
              }
            } catch (InterruptedException e) {
              Log.error("Interrupted:", e);
            }
          }

          // Check, may be new message added by addMessage(...) method
          if (messagesList.getFirst().equals(currentMessage)) {
            processMessage(currentMessage);
            messagesList.remove(currentMessage);
          }

        }
      }

    }

    Log.info("Time period over");
  }

  private void processMessage(final CalendarMessage message) {
    // Send message
    Log.info("----------------------------");
    Log.info("Sending message: from abonent = " + message.getSource() + ", to abonent = " + message.getDest() + ", send date =" + message.getSendDate() + ", message = " + message.getMessage());
    final Message msg = new Message();
    msg.setSourceAddress(message.getSource());
    msg.setDestinationAddress(message.getDest());
    msg.setMessageString(message.getMessage());
    outQueue.addOutgoingObject(new OutgoingObject(msg));

    // Remove message from DB
    Log.info("Removing message from database if exists");
    removeMessageFromDB(message);
    Log.info("----------------------------");
  }

  private void removeMessageFromDB(final CalendarMessage message) {
    if (!message.isExists())
      return;

    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = ConnectionPool.getConnection();
      ps = conn.prepareStatement(removeMessageSQL);
      ps.setInt(1, message.getId());

      ps.executeUpdate();
    } catch (SQLException e) {
      Log.error("Can't remove message: " , e);
    } finally {
      close(null, ps, conn);
    }
  }

  private boolean loadList() {
    Log.info("Loading messages list...");
    Connection conn = null;
    PreparedStatement ps = null;
    ResultSet rs = null;

    synchronized(messagesList) {
      final int previousSize = messagesList.size();

      try {
        conn = ConnectionPool.getConnection();
        ps = conn.prepareStatement(loadListSQL);

        ps.setTimestamp(1, new Timestamp(nextReloadTime.getTime()));
        ps.setInt(2, messagesList.getMaxSize() - messagesList.size());
        rs = ps.executeQuery();

        while (rs != null && rs.next())
          messagesList.add(new CalendarMessage(rs.getInt(1), rs.getString(2), rs.getString(3), rs.getTimestamp(4), rs.getString(5)));

        Log.info("Loading messages list ok.");
      } catch (SQLException e) {
        Log.error("Can't load messages list: ", e);
      } finally {
        close(rs, ps, conn);
      }

      final boolean newMessagesAdded = messagesList.size() - previousSize > 0;
      if (newMessagesAdded)
        messagesList.setMaxDate(messagesList.getLast().getSendDate());
      else
        messagesList.setMaxDate(nextReloadTime);

      return newMessagesAdded;
    }
  }

  private void close(final ResultSet rs, final PreparedStatement ps, final Connection conn) {
    try {
      if (rs != null)
        rs.close();
      if (ps != null)
        ps.close();
      if (conn != null) {
        conn.commit();
        conn.close();
      }
    } catch (SQLException e) {
      Log.error("Can't close: " , e);
    }
  }

  public CalendarMessagesList getMessagesList() {
    return messagesList;
  }

}
