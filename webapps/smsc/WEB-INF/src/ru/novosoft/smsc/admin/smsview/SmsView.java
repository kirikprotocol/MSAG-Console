package ru.novosoft.smsc.admin.smsview;

/**
 * Title:        SMSC Java Administration
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:      Novosoft
 * @author
 * @version 1.0
 */

import java.sql.*;
import javax.sql.*;

public class SmsView
{
  private DataSource ds = null;

  public void setDataSource(DataSource ds) {
    this.ds = ds;
  }

  public SmsSet getSmsSet(SmsQuery query) {
    System.out.println("From date: "+query.getFromDate().toString()+
                       " Till date: "+query.getTillDate().toString());
    SmsSet set = new SmsSet();
    try
    {
      Connection connection = ds.getConnection();
      String sql = prepareQueryString(query);
      System.out.println("SQL: "+sql);
      PreparedStatement stmt = connection.prepareStatement(sql);
      bindInput(stmt, query);
      ResultSet rs = stmt.executeQuery();
      while (rs.next()) {
        SmsRow row = new SmsRow();
        row.setDate(rs.getDate(2));
        row.setFrom(rs.getString(3));
        row.setTo(rs.getString(4));
        row.setStatus(rs.getInt(5));
        set.addRow(row);
      }
      rs.close();
      connection.close();
    }
    catch (Exception exc) {
      System.out.println("Operation with DB failed !");
      exc.printStackTrace();
    }
    return set;
  }

  private void bindInput(PreparedStatement stmt, SmsQuery query)
    throws SQLException
  {
    int pos=1;
    /*stmt.setString(pos++, query.getFromAddress());
    stmt.setString(pos++, query.getToAddress());*/
    if (query.getFromDateEnabled())
      stmt.setDate(pos++, new java.sql.Date(query .getFromDate().getTime()));
    if (query.getTillDateEnabled())
      stmt.setDate(pos++, new java.sql.Date(query .getTillDate().getTime()));
  }
  private String prepareQueryString(SmsQuery query) {
    String sql = "SELECT ID, SUBMIT_TIME, OA, DDA, ST, BODY_LEN, BODY FROM ";
    sql += (query.getStorageType() == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) ?
                "SMS_MSG":"SMS_ARC";
    sql += prepareWhereClause(query);
    sql += prepareOrderClause(query);
    return sql;
  }
  private String prepareWhereClause(SmsQuery query) {
    /*
    String where = " WHERE ";
    where += "OA = ? AND DDA = ?";
    if (query.getFromDateEnabled()) {
      where += " AND SUBMIT_TIME >= ?";
    }
    if (query.getTillDateEnabled()) {
      where += " AND SUBMIT_TIME <= ?";
    }
    return where;
    */
    String where = "";
    if (query.getFromDateEnabled()) {
      where += "WHERE SUBMIT_TIME >= ?";
    }
    if (query.getTillDateEnabled()) {
      if (query.getFromDateEnabled()) where += " AND";
      else where += " WHERE";
      where += " SUBMIT_TIME <= ?";
    }
    return where;
  }
  private String prepareOrderClause(SmsQuery query) {
    String order = query.getSortBy();
    if (order == null) return "";
    else if (order.equalsIgnoreCase("Date")) order="SUBMIT_TIME";
    else if (order.equalsIgnoreCase("Status")) order="ST";
    else if (order.equalsIgnoreCase("From")) order="OA";
    else if (order.equalsIgnoreCase("To")) order="DDA";
    else return "";
    return " ORDER BY "+order;
  }
};