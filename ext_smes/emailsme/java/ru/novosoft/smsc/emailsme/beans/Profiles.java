package ru.novosoft.smsc.emailsme.beans;

import ru.novosoft.smsc.emailsme.backend.*;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.util.conpool.NSConnectionPool;

import javax.servlet.http.HttpServletRequest;
import java.sql.*;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 15.09.2003
 * Time: 16:29:53
 * To change this template use Options | File Templates.
 */
public class Profiles extends SmeBean
{
  public static final int RESULT_EDIT = SmeBean.PRIVATE_RESULT + 0;
  public static final int RESULT_ADD = SmeBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = SmeBean.PRIVATE_RESULT + 2;

  private QueryResultSet resultSet = new EmptyResultSet();
  private int startPosition = 0;
  private String sort = null;
  private int pageSize = -1;
  private String edit = null;
  private String[] checked = new String[0];
  private Collection checkedSet = new HashSet();

  private String mbEdit = null;
  private String mbAdd = null;
  private String mbDelete = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (sort == null)
      sort = getSmeContext().getSort();
    else
      getSmeContext().setSort(sort);
    if (pageSize == -1)
      pageSize = getSmeContext().getPageSize();
    else
      getSmeContext().setPageSize(pageSize);

    checkedSet.addAll(Arrays.asList(checked));

    final NSConnectionPool connectionPool = getSmeContext().getConnectionPool();
    if (connectionPool != null) {
      try {
        resultSet = new ProfilesDataSource().query(connectionPool, new ProfilesQuery(getPageSizeInt(), new ProfilesFilter(null, null, -1, null), getSort(), getStartPositionInt()));
      } catch (SQLException e) {
        return error("Could not get profiles from db", e);
      }
    } else
      return result = error("Could not connect to SQL server.");

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbEdit != null) return RESULT_EDIT;
    if (mbAdd != null) return RESULT_ADD;
    if (mbDelete != null) return delete();

    return result;
  }

  private int delete()
  {
    final NSConnectionPool connectionPool = getSmeContext().getConnectionPool();
    if (connectionPool == null)
      return error("Could not connect to SQL server");

    Connection connection = null;
    logger.debug("delete " + checked.length + " profiles");
    try {
      connection = connectionPool.getConnection();
      PreparedStatement statement = connection.prepareStatement("delete from emlsme_profiles where address=?");
      for (int i = 0; i < checked.length; i++) {
        String checkedAddr = checked[i];
        statement.setString(1, checkedAddr);
        statement.executeUpdate();
      }
      connection.commit();
    } catch (SQLException e) {
      StringBuffer checkedAddrs = new StringBuffer("[");
      for (int i = 0; i < checked.length; i++) {
        if (i > 0)
          checkedAddrs.append(", ");
        checkedAddrs.append('"').append(checked[i]).append('"');
      }
      checkedAddrs.append(']');
      logger.error("Could not delete checked profiles " + checkedAddrs, e);
      return error("Could not delete checked profiles " + checkedAddrs, e);
    } finally {
      try {
        if (connection != null) connection.close();
      } catch (SQLException e) {
        logger.error("Could not close conection", e);
        error("Could not close conection", e);
      }
    }
    return RESULT_DONE;
  }

  public QueryResultSet getResultSet()
  {
    return resultSet;
  }

  public String getSort()
  {
    return sort;
  }

  public void setSort(String sort)
  {
    this.sort = sort;
  }

  public int getPageSizeInt()
  {
    return pageSize;
  }

  public void setPageSizeInt(int pageSize)
  {
    this.pageSize = pageSize;
  }

  public String getPageSize()
  {
    return String.valueOf(pageSize);
  }

  public void setPageSize(String pageSize)
  {
    try {
      this.pageSize = Integer.decode(pageSize).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid value for pagesize: " + pageSize);
    }
  }

  public int getStartPositionInt()
  {
    return startPosition;
  }

  public void setStartPositionInt(int startPosition)
  {
    this.startPosition = startPosition;
  }

  public String getStartPosition()
  {
    return String.valueOf(startPosition);
  }

  public void setStartPosition(String startPosition)
  {
    try {
      this.startPosition = Integer.decode(startPosition).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid value for start position: " + startPosition);
    }
  }

  public String getEdit()
  {
    return edit;
  }

  public void setEdit(String edit)
  {
    this.edit = edit;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String[] getChecked()
  {
    return checked;
  }

  public void setChecked(String[] checked)
  {
    this.checked = checked;
  }

  public boolean isChecked(String addr)
  {
    return checkedSet.contains(addr);
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public int getTotalSizeInt()
  {
    return resultSet.getTotalSize();
  }
}
