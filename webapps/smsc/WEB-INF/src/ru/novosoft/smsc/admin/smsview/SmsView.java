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
import java.io.UnsupportedEncodingException;

import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;

public class SmsView
{
  private static int   MAX_SMS_BODY_LENGTH    = 1650;
  private static short SMPP_SHORT_MESSAGE_TAG = 7;
  private static short SMPP_DATA_CODING_TAG   = 3;
  private static short SMPP_ESM_CLASS_TAG     = 2;

  private static short DATA_CODING_DEFAULT    = 0;    // 0
  private static short DATA_CODING_BINARY     = 4;    // BIT(2)
  private static short DATA_CODING_UCS2       = 8;    // BIT(3)
  private static short DATA_CODING_SMSC7BIT   = 0xf0; // 0xf0;

  private DataSource ds = null;

  public void setDataSource(DataSource ds) {
    this.ds = ds;
  }

  public SmsSet getSmsSet(SmsQuery query)
  {
    System.out.println("From date: "+query.getFromDate().toString()+
                       " Till date: "+query.getTillDate().toString());
    SmsSet set = new SmsSet();
    Connection connection = null;
    try
    {
      connection = ds.getConnection();
      if (connection == null) return set;
      String sql = prepareQueryString(query);
      //System.out.println("SQL: "+sql);
      PreparedStatement stmt = connection.prepareStatement(sql);
      bindInput(stmt, query);
      fetchRows(stmt, set);
      connection.close();
    }
    catch (Exception exc)
    {
      try { if (connection != null) connection.close(); }
      catch (Exception cexc) { cexc.printStackTrace(); }
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
    stmt.setString(pos++, getLikeExpression(query.getRouteId()));
    stmt.setString(pos++, getLikeExpression(query.getSrcSmeId()));
    stmt.setString(pos++, getLikeExpression(query.getDstSmeId()));
    if (query.getFromDateEnabled())
      stmt.setTimestamp(pos++,
        new java.sql.Timestamp(query .getFromDate().getTime()));
    if (query.getTillDateEnabled())
      stmt.setTimestamp(pos++,
        new java.sql.Timestamp(query .getTillDate().getTime()));
  }
  private void fetchRows(PreparedStatement stmt, SmsSet set)
    throws SQLException
  {
    String selectLargeBody = "SELECT BODY FROM SMS_ATCH WHERE ID=?";
    ResultSet rs = stmt.executeQuery();
    Connection connection = stmt.getConnection();
    PreparedStatement lbstmt = connection.prepareStatement(selectLargeBody);
    int fetchedCount = 0;
    while (rs.next() && SmsSet.MAX_SMS_FETCH_COUNT > fetchedCount++)
    {
      SmsRow row = new SmsRow();
      int pos=1;
      byte id[] = rs.getBytes(pos++);
      /*System.out.print("ID-size "+id.length+">> ");
      for (int i=0; i<id.length; i++) { System.out.print(id[i]); }
      System.out.println();*/
      row.setDate(rs.getTimestamp(pos++));
      row.setFrom(rs.getString(pos++));
      row.setTo(rs.getString(pos++));
      row.setStatus(rs.getInt(pos++));
      int bodyLen = rs.getInt(pos++);
      if (bodyLen <= 0) {
        row.setText("");
      }
      else if (bodyLen <= MAX_SMS_BODY_LENGTH) {
        byte body[] = rs.getBytes(pos);
        row.setText(convertBody(new ByteArrayInputStream(body, 0, bodyLen)));
      } else {
        ResultSet lbrs = null;
        try {
          lbstmt.setBytes(1, id);
          lbrs = lbstmt.executeQuery();
          Blob blob = lbrs.getBlob(1);
          row.setText("BLOB >> " +convertBody(blob.getBinaryStream()));
        } catch (Exception exc) {
            System.out.println("Retrive Blob from DB failed !");
            if (lbrs != null) lbrs.close();
            exc.printStackTrace();
        }
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
    where += " AND UPPER(ROUTE_ID) ";
    where += (needLikeExpression(query.getRouteId())) ? "LIKE ?":"= ?";
    where += " AND UPPER(SRC_SME_ID) ";
    where += (needLikeExpression(query.getSrcSmeId())) ? "LIKE ?":"= ?";
    where += " AND UPPER(DST_SME_ID) ";
    where += (needLikeExpression(query.getDstSmeId())) ? "LIKE ?":"= ?";
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
  private String convertBody(InputStream source)
  {
    String  message = "";
    int     textEncoding = DATA_CODING_DEFAULT;
    byte    esmClass = 0;
    int     textLen = 0;
    byte    text[] = null;

    System.out.println("Converting SMS body ...");
    try
    {
      DataInputStream stream = new DataInputStream(source);
      while (stream.available() > 0)
      {
        short tag = stream.readShort();
        int   len = stream.readInt();
        //System.out.println("Tag "+tag+" Len "+len);

        if (tag == SMPP_SHORT_MESSAGE_TAG) {
          byte msgText[] = new byte[textLen = len];
          stream.read(msgText, 0, textLen);
          text = msgText;
        } else if (tag == SMPP_DATA_CODING_TAG) {
          textEncoding = stream.readInt();
        } else if (tag == SMPP_ESM_CLASS_TAG) {
          esmClass = (byte)stream.readInt();
        } else {
          stream.skip(len);
        }
      }
      stream.close();

      if (text != null && (esmClass & 0x40) == 0x40) {
        DataInputStream input = new DataInputStream(
                                new ByteArrayInputStream(text, 0, textLen));
        int headerLen = (int)input.readByte();          // Byte ???
        textLen -= headerLen+1; input.skip(headerLen);  // +1 ???
        byte msgText[] = new byte[textLen];
        stream.read(msgText, 0, textLen);
        text = msgText;
      }

      if (text != null)
        message = decodeMessage(text, textLen, textEncoding);
    }
    catch (IOException exc)
    {
        System.out.println("SMS Body conversion failed !");
        exc.printStackTrace();
    }
    System.out.println("SMS body converted.");
    return message;
  }
  private String decodeMessage(byte text[], int len, int encoding)
    throws UnsupportedEncodingException
  {
    String message = "";
    if (encoding == DATA_CODING_DEFAULT) {          // ISO-LATIN-1
      message = new String(text, 0, len, "ISO-8859-1");
    } else if (encoding == DATA_CODING_SMSC7BIT) {  // US-ASCII
      message = new String(text, 0, len, "US-ASCII");
    } else if (encoding == DATA_CODING_UCS2) {      // UTF-16
      StringBuffer sb = new StringBuffer();
      for ( int i=0; i<len/2; ++i)
      {
        int x = ((((int)text[i*2])&0x0ff)) | ((((int)text[i*2+1])&0x0ff)<<8);
        sb.append("&#").append(x).append(';');
      }
      message = sb.toString();
    } else if (encoding == DATA_CODING_BINARY) { // DATA_CODING_BINARY
      message = "<< Binary data >>";
    } else {
      message = "<< Unsupported encoding ("+encoding+") !!! >>";
    }
    return message;
  }
};