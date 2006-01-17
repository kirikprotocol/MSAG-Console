package ru.novosoft.smsc.emailsme.beans;

import ru.sibinco.util.conpool.ConnectionPool;

import javax.servlet.http.HttpServletRequest;
import java.sql.*;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 15.09.2003
 * Time: 19:10:32
 * To change this template use Options | File Templates.
 */
public class Edit extends SmeBean
{
  private String addr = null;
  private String userid = null;
  private int dayLimit = -1;
  private String forward = null;

  private String mbDone = null;
  private String mbCancel = null;
  private boolean create = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!isInitialized()) {
      if (create) {
        addr = "";
        userid = "";
        dayLimit = 0;
        forward = "";
      } else {
        final ConnectionPool connectionPool = getSmeContext().getConnectionPool();
        if (connectionPool == null)
          return error("Could not connect to SQL server");

        Connection connection = null;
        try {
          connection = connectionPool.getConnection();
          PreparedStatement statement = connection.prepareStatement("select * from emlsme_profiles where address=?");
          statement.setString(1, addr);
          ResultSet resultSet = statement.executeQuery();
          if (!resultSet.next())
            return error("Unknown profile for addr \"" + addr + '"');
          userid = resultSet.getString("username");
          dayLimit = resultSet.getInt("daily_limit");
          forward = resultSet.getString("forward");
        } catch (SQLException e) {
          return error("Could not init from database", e);
        } finally {
          if (addr == null) addr = "";
          if (userid == null) userid = "";
          if (forward == null) forward = "";
          try {
            if (connection != null) connection.close();
          } catch (SQLException e) {
            logger.error("Couldn't close connection");
            error("Could not close connection", e);
          }
        }
      }
    }

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbDone != null) return done();
    if (mbCancel != null) return RESULT_DONE;
    return result;
  }

  private int done()
  {
    final ConnectionPool connectionPool = getSmeContext().getConnectionPool();
    if (connectionPool == null)
      return error("Could not connect to SQL server");

    Connection connection = null;
    try {
      connection = connectionPool.getConnection();
      if (create) {
        logger.debug("Create new profile: address=" + addr + ", username=" + userid + ", daily_limit=" + dayLimit + ", forward=" + forward);
        PreparedStatement statement = connection.prepareStatement("insert into emlsme_profiles (address, username, daily_limit, forward) values (?, ?, ?, ?)");
        statement.setString(1, addr);
        statement.setString(2, userid);
        statement.setInt(3, dayLimit);
        statement.setString(4, forward);
        int updatedRows = statement.executeUpdate();
        connection.commit();
        logger.debug("Inserted " + updatedRows + " rows");
      } else {
        logger.debug("Save changes: address=" + addr + ", username=" + userid + ", daily_limit=" + dayLimit + ", forward=" + forward);
        PreparedStatement statement = connection.prepareStatement("update emlsme_profiles set username=?, daily_limit=?, forward=? where address=?");
        statement.setString(1, userid);
        statement.setInt(2, dayLimit);
        statement.setString(3, forward);
        statement.setString(4, addr);
        int updatedRows = statement.executeUpdate();
        connection.commit();
        logger.debug("Updated " + updatedRows + " rows");
      }
    } catch (SQLException e) {
      logger.error("Could not update profile for addr \"" + addr + '"', e);
      return error("Could not update profile", e);
    } finally {
      try {
        if (connection != null) connection.close();
      } catch (SQLException e) {
        logger.error("Could not close connection", e);
        error("Could not close connection", e);
      }
    }

    return RESULT_DONE;
  }

  public String getAddr()
  {
    return addr;
  }

  public void setAddr(String addr)
  {
    this.addr = addr;
  }

  public String getUserid()
  {
    return userid;
  }

  public void setUserid(String userid)
  {
    this.userid = userid;
  }

  public int getDayLimitInt()
  {
    return dayLimit;
  }

  public void setDayLimitInt(int dayLimit)
  {
    this.dayLimit = dayLimit;
  }

  public String getDayLimit()
  {
    return String.valueOf(dayLimit);
  }

  public void setDayLimit(String dayLimit)
  {
    try {
      this.dayLimit = Integer.decode(dayLimit).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid value for day limit: " + dayLimit);
    }
  }

  public String getForward()
  {
    return forward;
  }

  public void setForward(String forward)
  {
    this.forward = forward;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public boolean isCreate()
  {
    return create;
  }

  public void setCreate(boolean create)
  {
    this.create = create;
  }
}
