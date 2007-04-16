package ru.sibinco.smsx.services.sponsored;

import ru.sibinco.smsx.services.ServiceProcessor;
import ru.sibinco.smsx.utils.BlockingQueue;
import ru.sibinco.smsx.utils.ConnectionPool;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.text.MessageFormat;
import java.sql.*;
import java.io.IOException;
import java.util.Calendar;

import com.logica.smpp.Data;

/**
 * User: artem
 * Date: 12.03.2007
 */

final class SponsoredRequestProcessorNew extends ServiceProcessor{

  private static org.apache.log4j.Category log = Category.getInstance(SponsoredRequestProcessorNew.class);

  private final BlockingQueue inQueue;
  private final SponsoredEngineNew sponsoredEngine;
  private final int totalWorkingTime = ( SponsoredServiceNew.Properties.Engine.TO_HOUR - SponsoredServiceNew.Properties.Engine.FROM_HOUR ) * 3600000;

  public SponsoredRequestProcessorNew(BlockingQueue messagesQueue, OutgoingQueue outQueue, Multiplexor multiplexor, SponsoredEngineNew sponsoredEngine) {
    super(log, outQueue, multiplexor);
    this.inQueue = messagesQueue;
    this.sponsoredEngine = sponsoredEngine;
  }

  public void iterativeWork() {
    try {
      processMessage((ParsedMessage)inQueue.getObject());
    } catch (Throwable e) {
      log.error("Error", e);
    }
  }

  public boolean processMessage(ParsedMessage msg) {
    final Message message = msg.getSourceMessage();
    final String abonent = message.getSourceAddress();

    try {

      log.info("=====================================================================================");
      log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());

      final AbonentSubscriptionInfo abonentSubscription = getAbonentSubscription(abonent);

      if (msg.getType() == ParsedMessage.UNSUBSCRIBE) {
        if (abonentSubscription.exists && abonentSubscription.count > 0) { // Abonent subscribed

          abonentSubscription.count = 0;
          insertInDatabase(abonentSubscription);

          sendResponse(message, Data.ESME_ROK);
          sendMessage(SponsoredServiceNew.Properties.SERVICE_ADDRESS, abonent, SponsoredServiceNew.Properties.Subscription.SUCCESSFULLY_MESSAGE_OFF);

        } else { // Abonent not subscribed
          sendResponse(message, Data.ESME_ROK);
          sendMessage(SponsoredServiceNew.Properties.SERVICE_ADDRESS, abonent, SponsoredServiceNew.Properties.Subscription.NOT_SUBSCRIBED_MESSAGE);
        }

      } else {
        abonentSubscription.count = msg.getCount();

        if (!abonentSubscription.exists) {
          abonentSubscription.todayCount = msg.getCount();

          final Calendar calendar = Calendar.getInstance();
          calendar.setTime(new java.util.Date());
          if (calendar.get(Calendar.HOUR_OF_DAY) > SponsoredServiceNew.Properties.Engine.TO_HOUR)
            calendar.setTimeInMillis(calendar.getTimeInMillis() + 24*3600000); // Add one day
          calendar.set(Calendar.HOUR_OF_DAY, SponsoredServiceNew.Properties.Engine.TO_HOUR);

          abonentSubscription.currentCount = msg.getCount() * (calendar.getTimeInMillis() - System.currentTimeMillis()) / totalWorkingTime;
          if (abonentSubscription.currentCount > msg.getCount())
            abonentSubscription.currentCount = msg.getCount();
          if (abonentSubscription.currentCount < 0)
            abonentSubscription.currentCount = 0;
        }

        if (!abonentSubscription.exists) {
          notification(abonent, msg.getCount());
          sponsoredEngine.abonentSubscribed(msg.getCount(), abonentSubscription.currentCount);
        }

        insertInDatabase(abonentSubscription);

        final MessageFormat mf = new MessageFormat((abonentSubscription.exists) ? SponsoredServiceNew.Properties.Subscription.SUCCESSFULLY_MESSAGE_TOMORROW : SponsoredServiceNew.Properties.Subscription.SUCCESSFULLY_MESSAGE);
        final String str = mf.format(new Object[]{new Integer(msg.getCount())});
        log.debug("Return text "+str);

        sendResponse(message, Data.ESME_ROK);
        sendMessage(SponsoredServiceNew.Properties.SERVICE_ADDRESS, abonent, str);
        return true;
      }

    } catch (Exception e) {
      log.error(e, e);
      sendResponse(message, Data.ESME_RSYSERR);
    }

    return true;
  }

  private void insertInDatabase(AbonentSubscriptionInfo info) throws SQLException {
    log.debug("adding info in database");
    Connection conn=null;
    PreparedStatement stmt=null;
    ResultSet rs=null;
    try {

      conn=ConnectionPool.getConnection();
      if(info.exists){//update
        stmt = conn.prepareStatement(SponsoredServiceNew.Properties.Subscription.SQL_UPDATE_ABONENT);
        stmt.setInt(1,info.count);
        stmt.setTimestamp(2,new Timestamp(System.currentTimeMillis()));
        stmt.setInt(3, info.todayCount);
        stmt.setString(4,info.abonent);
        stmt.executeUpdate();
        stmt.close();
      }else{//insert
        stmt = conn.prepareStatement(SponsoredServiceNew.Properties.Subscription.SQL_INSERT_ABONENT);
        stmt.setString(1,info.abonent);
        stmt.setInt(2,info.count);
        stmt.setTimestamp(3,new Timestamp(System.currentTimeMillis()));
        stmt.setInt(4, info.todayCount);
        stmt.setLong(5, info.currentCount);
        stmt.executeUpdate();
        stmt.close();
      }
    } finally {
      close(rs, stmt, conn);
    }

  }

  private AbonentSubscriptionInfo getAbonentSubscription(String abonent) throws SQLException {
    Connection conn=null;
    PreparedStatement stmt=null;
    ResultSet rs=null;
    try {
      conn=ConnectionPool.getConnection();
      stmt = conn.prepareStatement(SponsoredServiceNew.Properties.Subscription.SQL_GET_ABONENT);
      stmt.setString(1,abonent);
      rs=stmt.executeQuery();
      if (!rs.next())
        return new AbonentSubscriptionInfo(abonent);

      final AbonentSubscriptionInfo info = new AbonentSubscriptionInfo(rs.getString(1));
      info.count = rs.getInt(2);
      info.todayCount = rs.getInt(3);
      info.currentCount = rs.getInt(4);
      info.exists = true;
      return info;
    } finally {
      close(rs, stmt, conn);
    }
  }

  private void notification(String abonent,int NN) throws IOException {

    log.debug("Starting notification.Host "+SponsoredServiceNew.Properties.Subscription.NOTIFICATION_HOST+" port "+SponsoredServiceNew.Properties.Subscription.NOTIFICATION_PORT);
    SMSXProfileCommandHandler handler = new SMSXProfileCommandHandler();
    boolean res = false;

      handler.connect(SponsoredServiceNew.Properties.Subscription.NOTIFICATION_HOST,SponsoredServiceNew.Properties.Subscription.NOTIFICATION_PORT,60000);
      if(!handler.authentication(SponsoredServiceNew.Properties.Subscription.NOTIFICATION_USER,SponsoredServiceNew.Properties.Subscription.NOTIFICATION_PASSWORD)){
        log.debug("Authentication failed.");

      }else{
        String alterProfile = "alter profile "+abonent+" sponsored "+NN;
        SMSXProfileCommandHandler.Response resp = handler.sendCommand(alterProfile);
        if(!resp.isSuccess()){
          log.debug("Notification failed.Answer "+resp.getResponseLines()[resp.getResponseLines().length-1]+".");
        }else{
          log.debug("Finish notification successfully.");
          res=true;
        }
      }

      handler.close();


    if(!res){
      throw new IOException("Notification failed.");
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
      log.error("Can't close: " , e);
    }
  }

  private class AbonentSubscriptionInfo {
    private final String abonent;
    private int count;
    private int todayCount;
    private long currentCount;
    private boolean exists = false;

    public AbonentSubscriptionInfo(String abonentAddress) {
      this.abonent = abonentAddress;
    }
  }
}
