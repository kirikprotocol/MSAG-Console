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

import java.util.Vector;
import java.util.ArrayList;
import java.util.GregorianCalendar;
import java.util.TimeZone;

import java.io.IOException;
import java.io.UnsupportedEncodingException;

import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;

import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.admin.smsc_service.CancelMessageData;

public class SmsView
{
    private static int   MAX_SMS_BODY_LENGTH      = 1500;
    private static byte  INT_TAG_TYPE             = 0;
    private static short SMPP_ESM_CLASS_TAG       = 2;
    private static short SMPP_DATA_CODING_TAG     = 3;
    private static short SMPP_SHORT_MESSAGE_TAG   = 28;
    private static short SMPP_MESSAGE_PAYLOAD_TAG = 29;


    private static short DATA_CODING_DEFAULT    = 0;    // 0
    private static short DATA_CODING_BINARY     = 4;    // BIT(2)
    private static short DATA_CODING_UCS2       = 8;    // BIT(3)
    private static short DATA_CODING_SMSC7BIT   = 0xf0; // 0xf0;

    private DataSource ds = null;
    private Smsc smsc = null;

    public void setDataSource(DataSource ds) { this.ds = ds; }
    public void setSmsc(Smsc smsc) { this.smsc = smsc; }

    public SmsSet getSmsSet(SmsQuery query)
    {
      SmsSet set = new SmsSet();
      Connection connection = null;
      PreparedStatement stmt = null;
      try {
        connection = ds.getConnection();
        if (connection == null) return set;
        String sql = prepareQueryString(query);
        stmt = connection.prepareStatement(sql);
        bindInput(stmt, query);
        fetchRows(stmt, set, query.getRowsMaximum());
      } catch (Exception exc) {
        System.out.println("Operation with DB failed !");
        exc.printStackTrace();
      } finally {
          try { if (stmt != null) stmt.close(); connection.close(); }
          catch (Exception cexc) { cexc.printStackTrace(); }
      }
      return set;
    }

    public int delArchiveSmsSet(SmsSet set)
    {
      Connection connection = null;
      PreparedStatement stmt = null;
      int deleted = 0;
      try {
        connection = ds.getConnection();
        if (connection == null) return 0;
        String sql = "DELETE FROM SMS_ARC WHERE ID=?";
        stmt = connection.prepareStatement(sql);
        for (int i=0; i<set.getRowsCount(); i++) {
          SmsRow row = set.getRow(i);
          if (row != null) {
            stmt.setBytes(1, row.getId());
            deleted += stmt.executeUpdate();
            connection.commit();
          }
        }
      } catch (Exception exc) {
        System.out.println("Operation with DB failed !");
        exc.printStackTrace();
      } finally {
          try { if (stmt != null) stmt.close(); connection.close(); }
          catch (Exception cexc) { cexc.printStackTrace(); }
      }
      return deleted;
    }

    public int delOperativeSmsSet(SmsSet set)
    {
      if (smsc == null) return -1;
      int deleted = 0;

      Vector output = new Vector();
      for (int i=0; i<set.getRowsCount(); i++) {
        SmsRow row = set.getRow(i);
        if (row != null) {
          output.addElement(new CancelMessageData(
                  row.getIdString(), row.getOriginatingAddress(), row.getDestinationAddress()));
          deleted++;
        }
      }

      try {
        smsc.processCancelMessages(output);
      }
      catch (Exception exc) {
        System.out.println("Failed to cancel messages on SMSC!");
        exc.printStackTrace();
        return -1;
      }
      return deleted;
    }

    private boolean needExpression(String str) {
      return (str != null && str.length() != 0 &&
              !str.trim().equalsIgnoreCase("*"));
    }
    private boolean needLikeExpression(String str) {
      return (str.indexOf('*') >=0 || str.indexOf('?') >= 0);
    }
    private String getLikeExpression(String str) {
      return (needLikeExpression(str)) ?
              str.trim().replace('*', '%').replace('?', '_').toUpperCase() :
              str.trim().toUpperCase();
    }
    private void bindInput(PreparedStatement stmt, SmsQuery query)
      throws SQLException
    {
      int pos=1;
      if (needExpression(query.getSmsId())) {
          try {
            long id = Long.valueOf( query.getSmsId() ).longValue();
            String ids = Long.toHexString(id);
            StringBuffer sb = new StringBuffer(16);
            for( int i = 0; i < 16-ids.length(); i++ ) sb.append( '0' );
            sb.append( ids.toUpperCase() );
            stmt.setString(pos++, sb.toString());
          } catch (NumberFormatException ex) {
            throw new SQLException("Invalid numeric format");
          }
      }
      if (needExpression(query.getFromAddress()))
          stmt.setString(pos++, getLikeExpression(query.getFromAddress()));
      if (needExpression(query.getToAddress()))
          stmt.setString(pos++, getLikeExpression(query.getToAddress()));
      if (needExpression(query.getRouteId()))
          stmt.setString(pos++, getLikeExpression(query.getRouteId()));
      if (needExpression(query.getSrcSmeId()))
          stmt.setString(pos++, getLikeExpression(query.getSrcSmeId()));
      if (needExpression(query.getDstSmeId()))
          stmt.setString(pos++, getLikeExpression(query.getDstSmeId()));

      GregorianCalendar cal = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
      if (query.getFromDateEnabled()) {
        //java.util.Date fromDate = DateConvertor.convertLocalToGMT(query.getFromDate());
        //stmt.setTimestamp(pos++, new java.sql.Timestamp(fromDate.getTime()));
        stmt.setTimestamp(pos++,
                new java.sql.Timestamp(query.getFromDate().getTime()), cal);
      }
      if (query.getTillDateEnabled()) {
        //java.util.Date tillDate = DateConvertor.convertLocalToGMT(query.getTillDate());
        //stmt.setTimestamp(pos++, new java.sql.Timestamp(tillDate.getTime()));
        stmt.setTimestamp(pos++,
                new java.sql.Timestamp(query.getTillDate().getTime()), cal);
      }
    }

    private String prepareQueryString(SmsQuery query)
    {
      String sql =
              "SELECT ID, ST, SUBMIT_TIME, VALID_TIME, ATTEMPTS, LAST_RESULT, "+
              "LAST_TRY_TIME, NEXT_TRY_TIME, OA, DA, DDA, MR, SVC_TYPE, "+
              "DR, BR, SRC_MSC, SRC_IMSI, SRC_SME_N, DST_MSC, DST_IMSI, DST_SME_N, "+
              "ROUTE_ID, SVC_ID, PRTY, SRC_SME_ID, DST_SME_ID, "+
              "BODY_LEN, BODY FROM ";

      sql += (query.getStorageType() == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) ?
                  "SMS_MSG":"SMS_ARC";
      sql += prepareWhereClause(query);
      return sql;
    }

    private void addWherePart(ArrayList list, String field, String str) {
        if (needExpression(str))
            list.add("UPPER("+field+") "+((needLikeExpression(str)) ? "LIKE ?":"=?"));
    }
    private void addWherePartEQ(ArrayList list, String field, String str) {
        if (needExpression(str))
            list.add("UPPER("+field+")=?");
    }
    private String prepareWhereClause(SmsQuery query)
    {
      ArrayList list = new ArrayList();

      addWherePartEQ(list, "ID", query.getSmsId());
      addWherePart(list, "OA", query.getFromAddress());
      addWherePart(list, "DDA", query.getToAddress());
      addWherePart(list, "ROUTE_ID", query.getRouteId());
      addWherePart(list, "SRC_SME_ID", query.getSrcSmeId());
      addWherePart(list, "DST_SME_ID", query.getDstSmeId());

      if (query.getFromDateEnabled()) list.add("SUBMIT_TIME >=?");
      if (query.getTillDateEnabled()) list.add("SUBMIT_TIME <=?");

      String where = (list.size() > 0) ? " WHERE ":"";
      for (int i=0; i<list.size(); i++) {
        where += (String)list.get(i);
        if (i<list.size()-1) where += " AND ";
      }
      return where;
    }

    private void fetchRows(PreparedStatement stmt, SmsSet set, int rowsMaximum)
      throws SQLException
    {
      ResultSet rs = stmt.executeQuery();

      try
      {
        int fetchedCount = 0;
        while (rs.next() && rowsMaximum > fetchedCount++)
        {
          System.out.println("SMSVIEW: got row");
          SmsRow row = new SmsRow();
          int pos=1;
          byte id[] = rs.getBytes(pos++);
          row.setId(id);
          row.setStatus(rs.getInt(pos++));
          //row.setSubmitTime(DateConvertor.convertGMTToLocal(rs.getTimestamp(pos++)));
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
          int bodyLen = rs.getInt(pos++);
          if (bodyLen <= 0) {
            row.setText("<< No message >>");
          }
          else if (bodyLen <= MAX_SMS_BODY_LENGTH) {
            byte body[] = rs.getBytes(pos);
            if (body == null || body.length == 0) row.setText("<< No message (body null) >>");
            else row.setText(convertBody(new ByteArrayInputStream(body, 0, bodyLen)));
          }
          else {
            ResultSet lbrs = null;
            PreparedStatement lbstmt = null;
            try {
              String selectLargeBody = "SELECT BODY FROM SMS_ATCH WHERE ID=?";
              lbstmt = stmt.getConnection().prepareStatement(selectLargeBody);
              lbstmt.setBytes(1, id);
              lbrs = lbstmt.executeQuery();
              lbrs.next();
              Blob blob = lbrs.getBlob(1);
              row.setText("BLOB >> " +convertBody(blob.getBinaryStream()));
            } catch (Exception exc) {
                System.out.println("Retrive Blob from DB failed !");
                exc.printStackTrace();
            } finally {
                if (lbrs != null) lbrs.close();
                if (lbstmt != null) lbstmt.close();
            }
          }
          set.addRow(row);
        }
      } catch (Exception exc) {
        System.out.println("Operation with DB failed !");
        exc.printStackTrace();
        throw new SQLException(exc.getMessage());
      } finally {
        try { if (rs != null) rs.close(); }
        catch (Exception cexc) { cexc.printStackTrace(); }
      }
    }

    private String convertBody(InputStream source)
    {
      String  message = "";
      int     textEncoding = DATA_CODING_DEFAULT;
      byte    esmClass = 0;
      int     textLen = 0;
      byte    text[] = null;

      //System.out.println("Converting SMS body ...");
      try
      {
        DataInputStream stream = new DataInputStream(source);
        while (stream.available() > 0)
        {
          short tag = stream.readShort();
          byte  type  = (byte)((tag & (short)0xff00)>>8);
          type &= (short)0x00ff; tag &= (short)0x00ff;

          int   len = (type == INT_TAG_TYPE) ? 4:stream.readInt();
          //System.out.println("Tag: "+tag+" Type: "+type+" Len: "+len);
          if (tag == SMPP_SHORT_MESSAGE_TAG || tag == SMPP_MESSAGE_PAYLOAD_TAG) {
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
          int headerLen = (int)input.readByte();
          textLen -= headerLen+1; input.skip(headerLen);
          if (textLen > 0) {
            byte msgText[] = new byte[textLen];
            input.read(msgText, 0, textLen);
            text = msgText;
          }
          else text = null;
        }

        if (text != null)
          message = decodeMessage(text, textLen, textEncoding);
      }
      catch (IOException exc) {
          System.out.println("SMS Body conversion failed !");
          exc.printStackTrace();
      }
      //System.out.println("SMS body converted.");
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
          int x = ((((int)text[i*2])&0x0ff)<<8) | ((((int)text[i*2+1])&0x0ff));
          sb.append("&#").append(x).append(';');
        }
        message = sb.toString();
      } else if (encoding == DATA_CODING_BINARY) { // DATA_CODING_BINARY
        message = "<< Binary data >>";
      } else {
        message = "<< Unsupported encoding ("+encoding+") ! >>";
      }
      //System.out.println("Msg: "+message);
      return message;
    }
}

