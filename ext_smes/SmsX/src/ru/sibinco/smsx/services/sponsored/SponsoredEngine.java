package ru.sibinco.smsx.services.sponsored;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.network.OutgoingObject;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.sibinco.smsx.utils.ConnectionPool;
import ru.sibinco.smsx.utils.Service;

import java.sql.*;
import java.util.Calendar;

/**
 * User: artem
 * Date: 22.01.2007
 */

final class SponsoredEngine extends Service {

  private static final Category cat = Category.getInstance(SponsoredEngine.class);

  private final static int STATUS_SENT = -1;

  private final OutgoingQueue outQueue;

  public SponsoredEngine(final OutgoingQueue outQueue) {
    super(cat);
    this.outQueue = outQueue;
  }

  private OutgoingObject createOutgoingObject(Message message, int id) {
    final OutgoingObject object = new SponsoredOutgoingObject(message);
    object.setId(id);
    return object;
  }

  public synchronized void iterativeWork() {
    cat.debug("delivery task running");

    Calendar cal = Calendar.getInstance();
    int hour = cal.get(Calendar.HOUR_OF_DAY);


    if (hour >= SponsoredService.Properties.Engine.FROM_HOUR || hour < SponsoredService.Properties.Engine.TO_HOUR) {

      Connection con = null;
      PreparedStatement stmt1 = null;
      PreparedStatement stmt2 = null;
      PreparedStatement stmt3 = null;
      ResultSet rs = null;

      try {
        con = ConnectionPool.getConnection();

        stmt1 = con.prepareStatement(SponsoredService.Properties.Engine.PORTION_QUERY);
        stmt2 = con.prepareStatement(SponsoredService.Properties.Engine.UPDATE_DELIVERY_STATUS_QUERY);
        stmt3 = con.prepareStatement(SponsoredService.Properties.Engine.INSERT_STATS_QUERY);

        rs = stmt1.executeQuery();
        int counter = 0;
        while (rs.next() && counter++ < SponsoredService.Properties.Engine.DELIVERY_PORTION) {
          int deliveryId = rs.getInt(1);
          int msgId = rs.getInt(2);
          String msg = rs.getString(3);
          String abonent  = rs.getString(4);
          Message mData = new Message();
          mData.setSourceAddress(SponsoredService.Properties.SERVICE_ADDRESS);
          mData.setDestinationAddress(abonent);
          mData.setMessageString(msg);
          outQueue.addOutgoingObject(createOutgoingObject(mData, deliveryId));
          cat.debug(
              "message added to queue: abonent=" + abonent +
              " deliveryId=" + deliveryId +
              " messageId=" + msgId +
              " message='" + msg + "'"
          );

          stmt2.setInt(1, STATUS_SENT);
          stmt2.setInt(2, deliveryId);
          stmt2.executeUpdate();
          cat.debug("delivery status updated");

          stmt3.setString(1, abonent);
          stmt3.setInt(2, msgId);
          stmt3.setInt(3, deliveryId);
          stmt3.setTimestamp(4, new Timestamp(System.currentTimeMillis()));
          stmt3.setInt(5, STATUS_SENT);
          stmt3.executeUpdate();
          cat.debug("stat record inserted");


          try {
            wait(SponsoredService.Properties.Engine.SEND_INTERVAL);
          } catch (InterruptedException e) {
            break;
          }
        }

      } catch (SQLException sqle) {
        cat.debug("database error", sqle);
      } finally {
        close(null, stmt3, null);
        close(null, stmt2, null);
        close(rs, stmt1, con);
      }
    }

    try {
      wait(SponsoredService.Properties.Engine.REITERATION_INTERVAL);
    } catch (InterruptedException e) {
      cat.error(e);
    }
  }

  private static void close(final ResultSet rs, final PreparedStatement ps, final Connection conn) {
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
      cat.error("Can't close: " , e);
    }
  }
}
