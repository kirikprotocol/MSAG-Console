package ru.sibinco.smsx.services.sponsored;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.network.OutgoingObject;
import ru.sibinco.smsx.utils.ConnectionPool;

import java.sql.*;

/**
 * User: artem
 * Date: 26.01.2007
 */
final class SponsoredOutgoingObject extends OutgoingObject{

  private static final Category log = Category.getInstance(SponsoredEngine.class);

  public SponsoredOutgoingObject(Message outgoingMessage) {
    super(outgoingMessage);
  }

  public void changeStatus(int status) {
    Connection con = null;
    PreparedStatement stmt = null;
    final int deliveryId = getId();

    try {
      con = ConnectionPool.getConnection();

      stmt = con.prepareStatement(SponsoredService.Properties.Engine.UPDATE_STATS_STATUS_QUERY);
      stmt.setInt(1, status);
      stmt.setTimestamp(2, new Timestamp(System.currentTimeMillis()));
      stmt.setInt(3, deliveryId);
      stmt.executeUpdate();
      stmt.close();
      log.debug("updated status=" + status + " in stat record, deliveryId=" + deliveryId);

      stmt = con.prepareStatement(SponsoredService.Properties.Engine.DELETE_DELIVERY_QUERY);
      stmt.setInt(1, deliveryId);
      stmt.executeUpdate();
      log.debug("deleted delivery record");


    } catch (SQLException sqle) {
      log.debug("database error", sqle);
    } finally {
      close(null, stmt, con);
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
      log.error("Can't close: " , e);
    }
  }
}
