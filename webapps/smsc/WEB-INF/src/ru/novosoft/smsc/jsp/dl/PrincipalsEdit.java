package ru.novosoft.smsc.jsp.dl;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;

import java.security.Principal;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 22.09.2003
 * Time: 16:51:48
 */
public class PrincipalsEdit extends SmscBean
{
  private String address = null;
  private int max_lst = 0;
  private int max_el = 0;
  private boolean create = false;
  private boolean initialized = false;

  private String mbCancel = null;
  private String mbDone = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      if (!create) {
        Connection connection = null;
        try {
          connection = appContext.getConnectionPool().getConnection();
          PreparedStatement statement = connection.prepareStatement("select * from dl_principals where address=?");
          statement.setString(1, address);
          ResultSet resultSet = statement.executeQuery();
          if (resultSet.next()) {
            max_lst = resultSet.getInt("max_lst");
            max_el = resultSet.getInt("max_el");
          } else {
            logger.error("Principal \"" + address + "\" not found");
            return error("Principal \"" + address + "\" not found");
          }
        } catch (SQLException e) {
          logger.error("Could not get principal", e);
          return error("Could not get principal", e);
        } finally {
          try {
            connection.close();
          } catch (SQLException e) {
            logger.error("Could not close connection", e);
            return error("Could not close connection", e);
          }
        }
      }
    }

    return result;
  }

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return RESULT_DONE;
    if (mbDone != null)
      return done();

    return result;
  }

  private int done()
  {
    Connection connection = null;
    try {
      connection = appContext.getConnectionPool().getConnection();
      PreparedStatement statement;
      if (create)
        statement = connection.prepareStatement("insert into dl_principals (max_lst, max_el, address) values (?, ?, ?)");
      else
        statement = connection.prepareStatement("update dl_principals set max_lst=?, max_el=? where address=?");

      statement.setInt(1, max_lst);
      statement.setInt(2, max_el);
      if (create) {
        try {
          statement.setString(3, create ? new Mask(address).getNormalizedMask() : address);
        } catch (AdminException e) {
          logger.error("Invalid address \"" + address + "\"", e);
          return error("Invalid address \"" + address + "\"", e);
        }
      } else {
        statement.setString(3, address);
      }
      int updatedRows = statement.executeUpdate();
    } catch (SQLException e) {
      logger.error("Could not update or create principal", e);
      return error("Could not update or create principal", e);
    } finally {
      try {
        if (connection != null)
        {
          connection.commit();
          connection.close();
        }
      } catch (SQLException e) {
        logger.error("Could not close database connection", e);
        return error("Could not close database connection", e);
      }
    }
    return RESULT_DONE;
  }

  public String getAddress()
  {
    return address;
  }

  public void setAddress(String address)
  {
    this.address = address;
  }

  public int getMax_lstInt()
  {
    return max_lst;
  }

  public void setMax_lstInt(int max_lst)
  {
    this.max_lst = max_lst;
  }

  public int getMax_elInt()
  {
    return max_el;
  }

  public void setMax_elInt(int max_el)
  {
    this.max_el = max_el;
  }

  public String getMax_lst()
  {
    return String.valueOf(max_lst);
  }

  public void setMax_lst(String max_lst)
  {
    try {
      this.max_lst = Integer.decode(max_lst).intValue();
    } catch (NumberFormatException e) {
      logger.error("Incorrect value \"" + max_lst + "\" for maximum lists", e);
      error("Incorrect value \"" + max_lst + "\" for maximum lists", e);
    }
  }

  public String getMax_el()
  {
    return String.valueOf(max_el);
  }

  public void setMax_el(String max_el)
  {
    try {
      this.max_el = Integer.decode(max_el).intValue();
    } catch (NumberFormatException e) {
      logger.error("Incorrect value \"" + max_el + "\" for maximum elements", e);
      error("Incorrect value \"" + max_el + "\" for maximum elements", e);
    }
  }

  public boolean isCreate()
  {
    return create;
  }

  public void setCreate(boolean create)
  {
    this.create = create;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }
}
