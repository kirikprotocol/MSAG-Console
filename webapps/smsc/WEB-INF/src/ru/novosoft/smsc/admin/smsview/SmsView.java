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
    try {
      Connection connection = ds.getConnection();
      System.out.println("Got connection !");
      connection.close();
    } catch (Exception exc) {
      System.out.println("Get & close connection failed !");
      exc.printStackTrace();
    }
    return new SmsSet();
  }

  private String prepareQueryString(SmsQuery query) {
    String sql = "SELECT SUBMIT_TIME, OA, DDA, ST, BODY_LEN, BODY FROM ";
    sql += (query.getStorageType() == SmsQuery.SMS_OPERATIVE_STORAGE_TYPE) ?
                "SMS_MSG":"SMS_ARC";
    sql += prepareWhereClause(query); sql += prepareOrderClause(query);
    return sql + ";";
  }
  private String prepareWhereClause(SmsQuery query) {
    return " WHERE 1=1 ";
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