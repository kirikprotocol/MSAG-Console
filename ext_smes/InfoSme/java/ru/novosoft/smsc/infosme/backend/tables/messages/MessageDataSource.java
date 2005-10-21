package ru.novosoft.smsc.infosme.backend.tables.messages;

import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.admin.AdminException;

import javax.sql.DataSource;
import java.util.*;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.ResultSet;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.05.2004
 * Time: 20:11:57
 * To change this template use Options | File Templates.
 */
public class MessageDataSource implements ru.novosoft.smsc.jsp.util.tables.DataSource
{
  private final static String[] COLUMN_NAMES  = {"id", "abonent", "status", "sendDate", "message"};
  private final static String   BASE_SQL_QUERY = "SELECT id, abonent, state, send_date, message FROM ";

  private DataSource ds = null;
  private String tablePrefix = null;
  private String error = null;

  public MessageDataSource(DataSource ds, String tablePrefix) {
    this.ds = ds; this.tablePrefix = tablePrefix;
  }

  public QueryResultSet query(Query query)
  {
    error = null;
    QueryResultSetImpl results = null;
    Connection connection = null;
    PreparedStatement stmt = null;
    ResultSet rs = null;
    try
    {
      Vector sortOrders = query.getSortOrder();
      results = new QueryResultSetImpl(COLUMN_NAMES, sortOrders);
      String sortOrder = (sortOrders != null && sortOrders.size() == 1) ? (String)sortOrders.get(0):null;

      connection = ds.getConnection();
      if (connection == null)
        throw new AdminException("Failed to obtain connection to DB");

      MessageFilter filter = (MessageFilter)query.getFilter();
      String sql = prepareSqlQuery(filter, sortOrder);
      stmt = connection.prepareStatement(sql);
      bindQueryInput(filter, stmt);
      rs = stmt.executeQuery();

      int start = query.getStartPosition();
      int quantity = query.getExpectedResultsQuantity();
      int total = 0;
      GregorianCalendar cal = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
      while (rs.next())
      {
        if (start > total++) continue;

        if (quantity > 0) {
          long id = rs.getLong(1);
          String abonent = rs.getString(2);
          if (abonent == null || rs.wasNull()) abonent = "<unknown>";
          byte status = rs.getByte(3);
          java.sql.Timestamp sendDateTs = rs.getTimestamp(4, cal);
          java.util.Date sendDate = (sendDateTs == null) ? null : new Date(sendDateTs.getTime());
          String message = rs.getString(5);
          if (message == null || rs.wasNull()) message = "<empty>";
          results.add(new MessageDataItem(id, abonent, message, sendDate, status));
          quantity--;
        }
      }
      results.setTotalSize(total);
    }
    catch (Exception exc) {
      exc.printStackTrace();
      error = exc.getMessage();
    }
    finally {
      try { if (rs != null) rs.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
      try { if (stmt != null) stmt.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
      try {  if (connection != null) connection.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
    }
    return results;
  }

  public int delete(MessageFilter filter)
  {
    int deleted = 0;
    Connection connection = null;
    PreparedStatement stmt = null;

    try {
      connection = ds.getConnection();
      if (connection == null)
        throw new AdminException("Failed to obtain connection to DB");

      String sql = prepareSqlDelete(filter);
      stmt = connection.prepareStatement(sql);
      bindDeleteInput(filter, stmt);
      deleted = stmt.executeUpdate();
      connection.commit();
    }
    catch (Exception exc) {
      exc.printStackTrace();
      error = exc.getMessage();
    }
    finally {
      try { if (stmt != null) stmt.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
      try {  if (connection != null) connection.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
    }
    return deleted;
  }
  public int delete(String taskId, String[] toDelete)
  {
    int deleted = 0;
    Connection connection = null;
    PreparedStatement stmt = null;

    try {
      connection = ds.getConnection();
      if (connection == null)
        throw new AdminException("Failed to obtain connection to DB");

      String sql = prepareSqlDelete(taskId);
      stmt = connection.prepareStatement(sql);

      for (int i=0; i<toDelete.length; i++)
      {
        long id = 0;
        try { id = Long.parseLong(toDelete[i]); }
        catch (Exception e) { e.printStackTrace(); continue; }
        stmt.setLong(1, id);
        deleted += stmt.executeUpdate();
        connection.commit();
      }
    }
    catch (Exception exc) {
      exc.printStackTrace();
      error = exc.getMessage();
    }
    finally {
      try { if (stmt != null) stmt.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
      try {  if (connection != null) connection.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
    }
    return deleted;
  }

  public int resend(MessageFilter filter)
  {
    int resent = 0;
    Connection connection = null;
    PreparedStatement stmt = null;

    try {
      connection = ds.getConnection();
      if (connection == null)
        throw new AdminException("Failed to obtain connection to DB");

      String sql = prepareSqlResend(filter);
      stmt = connection.prepareStatement(sql);
      bindResendInput(filter, stmt);
      resent = stmt.executeUpdate();
      connection.commit();
    }
    catch (Exception exc) {
      exc.printStackTrace();
      error = exc.getMessage();
    }
    finally {
      try { if (stmt != null) stmt.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
      try {  if (connection != null) connection.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
    }
    return resent;
  }
  public int resend(String taskId, String[] toDelete)
  {
    int resent = 0;
    Connection connection = null;
    PreparedStatement stmt = null;

    try {
      connection = ds.getConnection();
      if (connection == null)
        throw new AdminException("Failed to obtain connection to DB");

      String sql = prepareSqlResend(taskId);
      stmt = connection.prepareStatement(sql);

      GregorianCalendar cal = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
      for (int i=0; i<toDelete.length; i++)
      {
        long id = 0;
        try { id = Long.parseLong(toDelete[i]); }
        catch (Exception e) { e.printStackTrace(); continue; }
        stmt.setByte(1, Message.MESSAGE_NEW_STATE);
        stmt.setTimestamp(2, new java.sql.Timestamp((new Date()).getTime()), cal);
        stmt.setLong(3, id);
        resent += stmt.executeUpdate();
        connection.commit();
      }
    }
    catch (Exception exc) {
      exc.printStackTrace();
      error = exc.getMessage();
    }
    finally {
      try { if (stmt != null) stmt.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
      try {  if (connection != null) connection.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
    }
    return resent;
  }

  public String getError() {
    return error;
  }

  private String prepareSqlQuery(MessageFilter filter, String sort) {
    return prepareSqlBase(BASE_SQL_QUERY, filter.getTaskId())+
           prepareWherePart(filter)+prepareOrderPart(sort);
  }
  private String prepareSqlDelete(MessageFilter filter) {
    return prepareSqlBase("DELETE FROM ", filter.getTaskId())+
           prepareWherePart(filter);
  }
  private String prepareSqlDelete(String taskId) {
    return prepareSqlBase("DELETE FROM ", taskId)+" WHERE id=?";
  }
  private String prepareSqlResend(String taskId) {
    return prepareSqlBase("UPDATE ", taskId)+" SET state=?, send_date=? WHERE id=?";
  }
  private String prepareSqlResend(MessageFilter filter) {
    return prepareSqlBase("UPDATE ", filter.getTaskId())+" SET state=?, send_date=?"+
           prepareWherePart(filter);
  }
  private String prepareSqlBase(String base, String taskId) {
    return base+tablePrefix+taskId;
  }
  private boolean needExpression(String str) {
    return (str != null && str.length() != 0 && !str.trim().equalsIgnoreCase("*"));
  }
  private String prepareWherePart(MessageFilter filter)
  {
    ArrayList list = new ArrayList();

    if (needExpression(filter.getAddress())) list.add("abonent=?");
    if (filter.getStatus() != Message.MESSAGE_UNDEFINED_STATE) list.add("state=?");
    if (filter.isFromDateEnabled()) list.add("send_date>=?");
    if (filter.isTillDateEnabled()) list.add("send_date<=?");

    if (list.size() <= 0) return "";
    String where = " WHERE ";
    for (int i = 0; i < list.size(); i++) {
      where += (String) list.get(i);
      if (i < list.size()-1) where += " AND ";
    }
    return where;
  }
  private String prepareOrderPart(String sort)
  {
    if (sort == null || sort.length() < 2) return "";
    String order = " ORDER BY ";
    boolean desc = sort.charAt(0) == '-';
    if (desc) sort = sort.substring(1);
    if (sort.equals(Message.SORT_BY_STATUS)) order+="state";
    else if (sort.equals(Message.SORT_BY_ABONENT)) order+="abonent";
    else order+="send_date";
    return order+" "+((desc) ? "DESC":"ASC");
  }
  private void bindInput(MessageFilter filter, PreparedStatement stmt, int index) throws SQLException
  {
    if (needExpression(filter.getAddress())) stmt.setString(index++, filter.getAddress());
    if (filter.getStatus() != Message.MESSAGE_UNDEFINED_STATE) stmt.setByte(index++, filter.getStatus());
    GregorianCalendar cal = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
    if (filter.isFromDateEnabled()) stmt.setTimestamp(index++, new java.sql.Timestamp(filter.getFromDate().getTime()), cal);
    if (filter.isTillDateEnabled()) stmt.setTimestamp(index++, new java.sql.Timestamp(filter.getTillDate().getTime()), cal);
  }
  private void bindQueryInput(MessageFilter filter, PreparedStatement stmt) throws SQLException  {
    bindInput(filter, stmt, 1);
  }
  private void bindDeleteInput(MessageFilter filter, PreparedStatement stmt) throws SQLException {
    bindInput(filter, stmt, 1);
  }
  private void bindResendInput(MessageFilter filter, PreparedStatement stmt) throws SQLException {
    stmt.setByte(1, Message.MESSAGE_NEW_STATE);
    GregorianCalendar cal = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
    stmt.setTimestamp(2, new java.sql.Timestamp((new java.util.Date()).getTime()), cal);
    bindInput(filter, stmt, 3);
  }
}
