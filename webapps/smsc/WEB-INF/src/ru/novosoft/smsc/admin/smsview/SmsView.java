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

import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;

public class SmsView
{
  private static int   MAX_SMS_BODY_LENGTH    = 1650;
  private static short SMPP_SHORT_MESSAGE_TAG = 7;
  private static short SMPP_DATA_CODING_TAG   = 3;

  private DataSource ds = null;

  public void setDataSource(DataSource ds) {
    this.ds = ds;
  }

  public SmsSet getSmsSet(SmsQuery query)
  {
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
      fetchRows(stmt, set);
      connection.close();
    }
    catch (Exception exc) {
      System.out.println("Operation with DB failed !");
      exc.printStackTrace();
    }
    return set;
  }

  private boolean needLikeExpression(String str)
  {
    return (str.indexOf('*') >=0 || str.indexOf('?') >= 0);
  }
  private String getLikeExpression(String str)
  {
    return (needLikeExpression(str)) ?
            str.replace('*', '%').replace('?', '_').toUpperCase() :
            str.toUpperCase();
  }
  private void bindInput(PreparedStatement stmt, SmsQuery query)
    throws SQLException
  {
    int pos=1;
    stmt.setString(pos++, getLikeExpression(query.getFromAddress()));
    stmt.setString(pos++, getLikeExpression(query.getToAddress()));
    if (query.getFromDateEnabled())
      stmt.setDate(pos++, new java.sql.Date(query .getFromDate().getTime()));
    if (query.getTillDateEnabled())
      stmt.setDate(pos++, new java.sql.Date(query .getTillDate().getTime()));
  }
  private void fetchRows(PreparedStatement stmt, SmsSet set)
    throws SQLException
  {
    String selectLargeBody = "SELECT BODY FROM SMS_ATCH WHERE ID=?";
    ResultSet rs = stmt.executeQuery();
    Connection connection = stmt.getConnection();
    PreparedStatement lbstmt = connection.prepareStatement(selectLargeBody);
    while (rs.next())
    {
      SmsRow row = new SmsRow();
      int pos=1;
      byte id[] = rs.getBytes(pos++);
      /*System.out.print("ID-size "+id.length+">> ");
      for (int i=0; i<id.length; i++) {
        System.out.print(id[i]);
      } System.out.println();*/
      row.setDate(rs.getDate(pos++));
      row.setFrom(rs.getString(pos++));
      row.setTo(rs.getString(pos++));
      row.setStatus(rs.getInt(pos++));
      int bodyLen = rs.getInt(pos++);
      if (bodyLen <= 0) {
        row.setText("");
      }
      else if (bodyLen <= MAX_SMS_BODY_LENGTH)
      {
        row.setText(
          convertBody(new ByteArrayInputStream(rs.getBytes(pos), 0, bodyLen), bodyLen)
        );
      }
      else
      {
        /*lbstmt.setBytes(1, id);
        ResultSet lbrs = lbstmt.executeQuery();*/
        row.setText("Large, len "+bodyLen);
      }
      set.addRow(row);
    }
    rs.close();
  }
  private String prepareQueryString(SmsQuery query)
  {
    String sql = "SELECT ID, SUBMIT_TIME, OA, DDA, ST, BODY_LEN, BODY FROM ";
    sql += (query.getStorageType() == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) ?
                "SMS_MSG":"SMS_ARC";
    sql += prepareWhereClause(query);
    sql += prepareOrderClause(query);
    return sql;
  }
  private String prepareWhereClause(SmsQuery query)
  {
    String where = " WHERE UPPER(OA) ";
    where += (needLikeExpression(query.getFromAddress())) ? "LIKE ?":"= ?";
    where += " AND UPPER(DDA) ";
    where += (needLikeExpression(query.getToAddress())) ? "LIKE ?":"= ?";
    if (query.getFromDateEnabled()) {
      where += " AND SUBMIT_TIME >= ?";
    }
    if (query.getTillDateEnabled()) {
      where += " AND SUBMIT_TIME <= ?";
    }
    return where;
  }
  private String prepareOrderClause(SmsQuery query)
  {
    String order = query.getSortBy();
    if (order == null) return "";
    else if (order.equalsIgnoreCase("Date")) order="SUBMIT_TIME";
    else if (order.equalsIgnoreCase("Status")) order="ST";
    else if (order.equalsIgnoreCase("From")) order="OA";
    else if (order.equalsIgnoreCase("To")) order="DDA";
    else return "";
    return " ORDER BY "+order;
  }
  private String convertBody(InputStream source, int length)
  {
    int     textEncoding = 0;
    boolean textFound = false;
    int     textLen = 0;
    byte    text[] = new byte[MAX_SMS_BODY_LENGTH];

    try
    {
      DataInputStream stream = new DataInputStream(source);
      while (stream.available() > 0)
      {
        short tag = stream.readShort();
        int   len = stream.readInt();
        System.out.println("Tag "+tag+" Len "+len);
        if (tag == SMPP_SHORT_MESSAGE_TAG)
        {
          stream.read(text, 0, len);
          textLen = len; textFound = true;
        }
        if (tag == SMPP_DATA_CODING_TAG)
        {
          textEncoding = stream.readInt();
        }
        else stream.skip(len);
      }
      stream.close();
    } catch (IOException exc) {
        System.out.println("SMS Body conversion failed !");
        exc.printStackTrace();
    }
    return ((textFound) ? new String(text, 0, textLen) : "");
  }
};