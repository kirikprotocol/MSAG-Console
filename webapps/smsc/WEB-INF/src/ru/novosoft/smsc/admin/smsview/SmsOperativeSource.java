package ru.novosoft.smsc.admin.smsview;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;

import javax.sql.DataSource;
import java.sql.*;
import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.util.GregorianCalendar;
import java.util.TimeZone;
import java.util.ArrayList;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 27.01.2004
 * Time: 16:36:55
 * To change this template use Options | File Templates.
 */
public class SmsOperativeSource extends SmsSource
{
  private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(Class.class);

  private final static int    MAX_SMS_BODY_LENGTH = 1500;
  private final static String SELECT_LARGE_BODY =
      "SELECT BODY FROM SMS_ATCH WHERE ID=?";
  private final static String BASE_SQL_QUERY =
      "SELECT ID, ST, SUBMIT_TIME, VALID_TIME, ATTEMPTS, LAST_RESULT, " +
      "LAST_TRY_TIME, NEXT_TRY_TIME, OA, DA, DDA, MR, SVC_TYPE, " +
      "DR, BR, SRC_MSC, SRC_IMSI, SRC_SME_N, DST_MSC, DST_IMSI, DST_SME_N, " +
      "ROUTE_ID, SVC_ID, PRTY, SRC_SME_ID, DST_SME_ID, " +
      "BODY_LEN, BODY FROM SMS_MSG";

  private DataSource ds = null;
  public void setDataSource(DataSource ds) {
    this.ds = ds;
  }

  public SmsSet getSmsSet(SmsQuery query) throws AdminException
  {
    SmsSet set = new SmsSet();
    Connection connection = null;
    PreparedStatement stmt = null;
    try {
      connection = ds.getConnection();
      if (connection == null)
        throw new AdminException("Failed to obtain connection to DB");
      String sql = prepareQueryString(query);
      stmt = connection.prepareStatement(sql);
      bindInput(stmt, query);
      fetchRows(stmt, set, query.getRowsMaximum());
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException("Failed to get sms set. Details: "+exc.getMessage());
    } finally {
      try { if (stmt != null) stmt.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
      try {  if (connection != null) connection.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
    }
    return set;
  }

  public int getSmsCount(SmsQuery query) throws AdminException
  {
    int count = 0;
    Connection connection = null;
    PreparedStatement stmt = null;
    ResultSet rs = null;
    try {
      connection = ds.getConnection();
      if (connection == null)
        throw new AdminException("Failed to obtain connection to DB");
      String sql = prepareCountString(query);
      stmt = connection.prepareStatement(sql);
      bindInput(stmt, query);
      rs = stmt.executeQuery();
      if (rs == null || !rs.next()) return 0;
      count = rs.getInt(1);
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException("Select count failed. Details: "+exc.getMessage());
    } finally {
      try { if (rs != null) rs.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
      try { if (stmt != null) stmt.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
      try {  if (connection != null) connection.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
    }
    return count;
  }

  /* ------------------ Private implementation ------------------ */

  private boolean needExpression(String str) {
    return (str != null && str.length() != 0 && !str.trim().equalsIgnoreCase("*"));
  }
  private boolean needLikeExpression(String str) {
    return (str.indexOf('*') >= 0 || str.indexOf('?') >= 0);
  }
  private String getLikeExpression(String str) {
    return (needLikeExpression(str)) ? str.trim().replace('*', '%').replace('?', '_') : str.trim();
  }

  private void bindInput(PreparedStatement stmt, SmsQuery query)
      throws SQLException, AdminException
  {
    int pos = 1;
    if (needExpression(query.getSmsId())) {
      try {
        stmt.setLong(pos++, Long.valueOf(query.getSmsId()).longValue());
      } catch (NumberFormatException ex) {
        throw new SQLException("Invalid numeric format for sms id");
      }
    }
    if (needExpression(query.getFromAddress()))
      stmt.setString(pos++, getLikeExpression((new Mask(query.getFromAddress())).getNormalizedMask()));
    if (needExpression(query.getToAddress()))
      stmt.setString(pos++, getLikeExpression((new Mask(query.getToAddress())).getNormalizedMask()));
    if (needExpression(query.getRouteId()))
      stmt.setString(pos++, getLikeExpression(query.getRouteId()));
    if (needExpression(query.getSrcSmeId()))
      stmt.setString(pos++, getLikeExpression(query.getSrcSmeId()));
    if (needExpression(query.getDstSmeId()))
      stmt.setString(pos++, getLikeExpression(query.getDstSmeId()));

    GregorianCalendar cal = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
    if (query.getFromDateEnabled()) {
      stmt.setTimestamp(pos++, new java.sql.Timestamp(query.getFromDate().getTime()), cal);
    }
    if (query.getTillDateEnabled()) {
      stmt.setTimestamp(pos++, new java.sql.Timestamp(query.getTillDate().getTime()), cal);
    }
  }

  private String prepareQueryString(SmsQuery query)
  {
    String sql = BASE_SQL_QUERY + prepareWhereClause(query);
    logger.debug(sql);
    return sql;
  }

  private String prepareCountString(SmsQuery query)
  {
    String sql = "SELECT COUNT(*) FROM SMS_MSG " + prepareWhereClause(query);
    logger.debug(sql);
    return sql;
  }

  private void addWherePart(ArrayList list, String field, String str)
  {
    if (needExpression(str)) list.add(field + ((needLikeExpression(str)) ? " LIKE ?" : "=?"));
  }

  private void addWherePartEQ(ArrayList list, String field, String str)
  {
    if (needExpression(str)) list.add(field + "=?");
  }

  private String prepareWhereClause(SmsQuery query)
  {
    ArrayList list = new ArrayList();

    addWherePartEQ(list, "ID", query.getSmsId());
    addWherePart  (list, "OA", query.getFromAddress());
    addWherePart  (list, "DDA", query.getToAddress());
    addWherePart  (list, "ROUTE_ID", query.getRouteId());
    addWherePart  (list, "SRC_SME_ID", query.getSrcSmeId());
    addWherePart  (list, "DST_SME_ID", query.getDstSmeId());

    if (query.getFromDateEnabled()) list.add("SUBMIT_TIME >=?");
    if (query.getTillDateEnabled()) list.add("SUBMIT_TIME <=?");

    String where = (list.size() > 0) ? " WHERE " : " WHERE ID>0";
    for (int i = 0; i < list.size(); i++) {
      where += (String) list.get(i);
      if (i < list.size() - 1) where += " AND ";
    }
    return where;
  }

  private InputStream fetchRowFeilds(ResultSet rs, SmsRow row)
      throws SQLException, AdminException
  {
    int pos = 1;
    long id = rs.getLong(pos++);
    row.setId(id);
    row.setStatus(rs.getInt(pos++));
    GregorianCalendar cal = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
    row.setSubmitTime(rs.getTimestamp(pos++, cal));
    row.setValidTime(rs.getTimestamp(pos++, cal));
    row.setAttempts(rs.getInt(pos++));
    row.setLastResult(rs.getInt(pos++));
    row.setLastTryTime(rs.getTimestamp(pos++, cal));
    row.setNextTryTime(rs.getTimestamp(pos++, cal));
    row.setOriginatingAddress(rs.getString(pos++));
    row.setDestinationAddress(rs.getString(pos++));
    row.setDealiasedDestinationAddress(rs.getString(pos++));
    row.setMessageReference(rs.getInt(pos++));
    row.setServiceType(rs.getString(pos++));
    row.setDeliveryReport(rs.getShort(pos++));
    row.setBillingRecord(rs.getShort(pos++));
    SmsDescriptor origDescr = new SmsDescriptor(
        rs.getString(pos++),
        rs.getString(pos++),
        rs.getInt(pos++)
    );
    row.setOriginatingDescriptor(origDescr);
    SmsDescriptor destDescr = new SmsDescriptor(
        rs.getString(pos++),
        rs.getString(pos++),
        rs.getInt(pos++)
    );
    row.setDestinationDescriptor(destDescr);
    row.setRouteId(rs.getString(pos++));
    row.setServiceId(rs.getInt(pos++));
    row.setPriority(rs.getInt(pos++));
    row.setSrcSmeId(rs.getString(pos++));
    row.setDstSmeId(rs.getString(pos++));

    byte body[] = null;
    InputStream is = null;

    int bodyLen = rs.getInt(pos++);
    if (bodyLen <= 0) {
      row.setText("<< No message >>");
    } else if (bodyLen <= MAX_SMS_BODY_LENGTH) {
      body = rs.getBytes(pos);
      if (body == null || body.length == 0)
        row.setText("<< No message (body null) >>");
      else
        is = new ByteArrayInputStream(body, 0, bodyLen);
    } else {
      ResultSet lbrs = null;
      PreparedStatement lbstmt = null;
      try {
        lbstmt = rs.getStatement().getConnection().prepareStatement(SELECT_LARGE_BODY);
        lbstmt.setLong(1, row.getId());
        lbrs = lbstmt.executeQuery();
        if (lbrs != null && lbrs.next()) {
          Blob blob = lbrs.getBlob(1);
          body = blob.getBytes(1, bodyLen); // 1 or 0 ???
          is = new ByteArrayInputStream(body, 0, bodyLen);
        }
        else row.setText("<< No message (Access to BLOB failed) >>");
      } catch (Exception exc) {
        exc.printStackTrace();
        throw new SQLException("Retrive Blob from operative storage failed. Details: "+exc.getMessage());
      } finally {
        try { if (lbrs != null) lbrs.close(); }
        catch (Exception cexc) { cexc.printStackTrace(); }
        try { if (lbstmt != null) lbstmt.close(); }
        catch (Exception cexc) { cexc.printStackTrace(); }
      }
    }

    return is;
  }

  private void fetchRows(PreparedStatement stmt, SmsSet set, int rowsMaximum)
      throws SQLException
  {
    ResultSet rs = stmt.executeQuery();

    try {
      int fetchedCount = 0;
      while (rs.next() && rowsMaximum > fetchedCount++) {
        SmsRow row = new SmsRow();
        InputStream is = fetchRowFeilds(rs, row);
        if (is != null) parseBody(is, row);
        set.addRow(row);
      }
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new SQLException("Operation with operative storage failed. Details: "+exc.getMessage());
    } finally {
      try { if (rs != null) rs.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
    }
  }

  /* Depricated. Implemented in SmsSet & SmsViewFormBean
  public SmsRow getSms(String id) throws AdminException
  {
    SmsRow row = new SmsRow();
    Connection connection = null;
    PreparedStatement stmt = null;
    try {
      connection = ds.getConnection();
      if (connection == null)
        throw new AdminException("Failed to obtain connection to DB");
      String sql = BASE_SQL_QUERY + " WHERE ID=" + id;
      stmt = connection.prepareStatement(sql);
      ResultSet rs = stmt.executeQuery();
      if (!rs.next())
        throw new AdminException("Sms #" + id + " no longer exists");
      InputStream is = fetchRowFeilds(rs, row);
      if (is != null) parseBody(is, row);
    } catch (Exception exc) {
      System.out.println("Operation with DB failed !");
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try {
        if (stmt != null) stmt.close();
        if (connection != null) connection.close();
      } catch (Exception cexc) {
        cexc.printStackTrace();
      }
    }
    return row;
  }*/

}
