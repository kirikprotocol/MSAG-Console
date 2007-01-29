package ru.sibinco.smsx.services;

import org.apache.log4j.Category;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class Storable {

  protected static void close(final Category log, final ResultSet rs, final PreparedStatement ps, final Connection conn) {
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
