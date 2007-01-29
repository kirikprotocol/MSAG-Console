package ru.sibinco.smsx.services.calendar;

import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.network.OutgoingObject;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.sibinco.smsx.utils.Service;

import java.sql.SQLException;
import java.util.Date;
import java.util.Iterator;

/**
 * User: artem
 * Date: Jul 31, 2006
 */

final class CalendarEngine extends Service {
  private static final org.apache.log4j.Category log = org.apache.log4j.Category.getInstance(CalendarEngine.class);

  private final  CalendarMessagesList messagesList;

  private final Object newMessageNotifier = new Object();

  private final Date nextReloadTime = new Date();
  private final OutgoingQueue outQueue;

  public CalendarEngine(final OutgoingQueue outQueue, final CalendarMessagesList messagesList) {
    super(log);

    if (outQueue == null)
      throw new InitializationException("CalendarEngine: outgoing queue is not specified");

    this.outQueue = outQueue;

    this.messagesList = messagesList;
    messagesList.addNewMessageNotifier(newMessageNotifier);
  }

  protected void doBeforeStart() {
    loadList();
  }

  public synchronized void iterativeWork() {
    log.info("=====================================================================================");
    nextReloadTime.setTime(System.currentTimeMillis() + CalendarService.Properties.CALENDAR_ENGINE_WORKING_INTERVAL);
    log.info("Start new time period. End time= " + nextReloadTime.toString());

    boolean hasDBMessages = true;

    while (System.currentTimeMillis() < nextReloadTime.getTime()) {
      log.info("Process messages list.");

      if (hasDBMessages && messagesList.isEmpty())
        hasDBMessages = loadList();

      if (messagesList.isEmpty()) {
        log.info("Messages list is empty. Waiting for the end of period or new messages...");
        try {
          synchronized(newMessageNotifier) {
            newMessageNotifier.wait(nextReloadTime.getTime() - System.currentTimeMillis());
          }
        } catch (InterruptedException e) {
          log.error("Interrupted:", e);
        } catch (IllegalArgumentException e) {
          log.error("Interrupted:", e);
        }
      }

      while (!messagesList.isEmpty()) {
        CalendarMessage currentMessage = messagesList.getFirst();
        log.info("Processing new message from list");
        if (currentMessage != null) {
          // Waiting for current message send time
          final long waitTime = currentMessage.getSendDate().getTime() - System.currentTimeMillis();
          if (waitTime > 0) {
            log.info("Next message send time = " + currentMessage.getSendDate() + "; waiting...");
            try {
              synchronized(newMessageNotifier) {
                newMessageNotifier.wait(waitTime);
              }
            } catch (InterruptedException e) {
              log.error("Interrupted:", e);
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

    log.info("Time period over");
  }

  private void processMessage(final CalendarMessage message) {
    // Send message
    log.info("----------------------------");
    log.info("Sending message: from abonent = " + message.getSource() + ", to abonent = " + message.getDest() + ", send date =" + message.getSendDate() + ", message = " + message.getMessage());
    final Message msg = new Message();
    msg.setSourceAddress(message.getSource());
    msg.setDestinationAddress(message.getDest());
    msg.setMessageString(message.getMessage());
    outQueue.addOutgoingObject(new OutgoingObject(msg));

    // Remove message from DB
    log.info("Removing message from database if exists");
    try {
      message.remove();
    } catch (SQLException e) {
      log.error("Can't remove message: " , e);
    }
    log.info("----------------------------");
  }


  private boolean loadList() {
    log.info("Loading messages list...");

    synchronized(messagesList) {
      final int previousSize = messagesList.size();

      try {

        for (Iterator iter = CalendarMessage.loadList(nextReloadTime, messagesList.getMaxSize() - messagesList.size()).iterator(); iter.hasNext();)
          messagesList.add((CalendarMessage)iter.next());

        log.info("Loading messages list ok.");
      } catch (SQLException e) {
        log.error("Can't load messages list: ", e);
      }

      final boolean newMessagesAdded = messagesList.size() - previousSize > 0;
      if (newMessagesAdded)
        messagesList.setMaxDate(messagesList.getLast().getSendDate());
      else
        messagesList.setMaxDate(nextReloadTime);

      return newMessagesAdded;
    }
  }

  public CalendarMessagesList getMessagesList() {
    return messagesList;
  }

}
