package ru.sibinco.smsx.services.sponsored;

import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.sibinco.smsx.services.ServiceProcessor;
import ru.sibinco.smsx.utils.BlockingQueue;
import ru.sibinco.smsx.utils.ConnectionPool;

import java.io.IOException;
import java.sql.*;
import java.text.MessageFormat;

/**
 * User: artem
 * Date: 22.01.2007
 */
final class SponsoredRequestProcessor extends ServiceProcessor{

  private static org.apache.log4j.Category log = Category.getInstance(SponsoredRequestProcessor.class);

  private final BlockingQueue inQueue;

  public SponsoredRequestProcessor(BlockingQueue messagesQueue, OutgoingQueue outQueue, Multiplexor multiplexor) {
    super(log, outQueue, multiplexor);
    this.inQueue = messagesQueue;
  }

  public void iterativeWork() {
    try {
      processMessage((Message)inQueue.getObject());
    } catch (Throwable e) {
      log.error("Error", e);
    }
  }

  public boolean processMessage(Message message) {
    try {

      String abonent = message.getSourceAddress();
      String text = message.getMessageString();

      log.info("=====================================================================================");
      log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());

      String num=text.split(" ")[1];
      int NN = Integer.parseInt(num);

      notification(abonent, NN);
      insertInDatabase(abonent, NN);

      MessageFormat mf = new MessageFormat(SponsoredService.Properties.Subscription.SUCCESSFULLY_MESSAGE);
      String str = mf.format(new Object[]{new Integer(NN)});
      log.debug("Return text "+str);
      sendMessage(SponsoredService.Properties.SERVICE_ADDRESS, abonent, str);

    } catch (Exception e) {
      log.error(e);
      sendResponse(message, Data.ESME_RSYSERR);
    }

    return true;
  }

  private void insertInDatabase(String abonent,int NN) throws SQLException {
    log.debug("adding info in database");
    Connection conn=null;
    PreparedStatement stmt=null;
    ResultSet rs=null;
    try {
      conn=ConnectionPool.getConnection();
      stmt = conn.prepareStatement(SponsoredService.Properties.Subscription.SQL_GET_ABONENT);
      stmt.setString(1,abonent);
      rs=stmt.executeQuery();
      boolean exist = rs.next();
      rs.close();
      stmt.close();
      if(exist){//update
        stmt = conn.prepareStatement(SponsoredService.Properties.Subscription.SQL_UPDATE_ABONENT);
        stmt.setInt(1,NN);
        stmt.setTimestamp(2,new Timestamp(System.currentTimeMillis()));
        stmt.setString(3,abonent);
        stmt.executeUpdate();
        stmt.close();
      }else{//insert
        stmt = conn.prepareStatement(SponsoredService.Properties.Subscription.SQL_INSERT_ABONENT);
        stmt.setString(1,abonent);
        stmt.setInt(2,NN);
        stmt.setTimestamp(3,new Timestamp(System.currentTimeMillis()));
        stmt.executeUpdate();
        stmt.close();
      }

    } finally {
      close(rs, stmt, conn);
    }

  }

  private void notification(String abonent,int NN) throws IOException {
    log.debug("Starting notification.Host "+SponsoredService.Properties.Subscription.NOTIFICATION_HOST+" port "+SponsoredService.Properties.Subscription.NOTIFICATION_PORT);
    SMSXProfileCommandHandler handler = new SMSXProfileCommandHandler();
    boolean res = false;

      handler.connect(SponsoredService.Properties.Subscription.NOTIFICATION_HOST,SponsoredService.Properties.Subscription.NOTIFICATION_PORT,60000);
      if(!handler.authentication(SponsoredService.Properties.Subscription.NOTIFICATION_USER,SponsoredService.Properties.Subscription.NOTIFICATION_PASSWORD)){
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

}
