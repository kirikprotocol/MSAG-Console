package com.eyeline.sponsored.ds;

import com.eyeline.sponsored.ds.DataSourceException;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * User: artem
 * Date: 31.01.2008
 */

public abstract class AbstractDBResultSetImpl<T> implements com.eyeline.sponsored.ds.ResultSet<T> {
  protected final ResultSet rs;
  protected final PreparedStatement ps;

  protected AbstractDBResultSetImpl(ResultSet rs, PreparedStatement ps) {
    this.rs = rs;
    this.ps = ps;
  }

  public boolean next() throws DataSourceException {
    try {
      return rs.next();
    } catch (SQLException e) {
      throw new DataSourceException(e);
    }
  }

  public void close() throws DataSourceException {
    try {
      if (rs != null)
        rs.close();
      if (ps != null)
        ps.close();
    } catch (SQLException e) {
      throw new DataSourceException(e);
    }        
  }
}
