package ru.sibinco.smsx.utils;

import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;

import java.sql.SQLException;
import java.io.InputStream;
import java.io.IOException;
import java.io.File;

/**
 * User: artem
 * Date: 14.05.2008
 */

public class DBDataSource {

  private final PropertiesConfig sql;
  private final String prefix;

  protected DBDataSource(InputStream is, String prefix) throws DataSourceException {
    try {
      this.sql = new PropertiesConfig();
      this.sql.load(is);

    } catch (IOException e) {
      throw new DataSourceException(e);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException e) {
          throw new DataSourceException(e);
        }
    }
    this.prefix = prefix;
  }

  protected DBDataSource(String sqlFile, String prefix) throws DataSourceException {
    try {
      this.sql = new PropertiesConfig();
      this.sql.load(new File(sqlFile));

    } catch (ConfigException e) {
      throw new DataSourceException(e);
    }
    this.prefix = prefix;
  }

  protected String getSql(java.lang.String string) throws DataSourceException {
    try {
      return sql.getString(prefix + string);
    } catch (ConfigException e) {
      throw new DataSourceException("Can't find sql: " + prefix + string);
    }
  }

  protected static void close(java.sql.ResultSet rs, java.sql.PreparedStatement ps, java.sql.Connection conn) {
    if (rs != null)
      try {
        rs.close();
      } catch (SQLException e) {
      }
    if (ps != null)
      try {
        ps.close();
      } catch (SQLException e) {
      }
    if (conn != null)
      try {
        conn.close();
      } catch (SQLException e) {
      }
  }
}
