package com.eyeline.sponsored.ds;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class AbstractDBTransaction {

  protected final Connection conn;

  public AbstractDBTransaction(Connection conn) {
    this.conn = conn;
  }

  public static void close(java.sql.ResultSet rs, PreparedStatement ps) {
    try {
      if (rs != null) {
        rs.close();
      }
    } catch (SQLException e) {
    }
    try {
      if (ps != null) {
        ps.close();
      }
    } catch (SQLException e) {
    }
  }

  public void commit() throws DataSourceException {
    try {
      conn.commit();
    } catch (SQLException ex) {
      throw new DataSourceException(ex);
    }
  }

  public void rollback() throws DataSourceException {
    try {
      conn.rollback();
    } catch (SQLException ex) {
      throw new DataSourceException(ex);
    }
  }

  public void close() {
    try {
      conn.close();
    } catch (SQLException ex) {
    }
  }
}
