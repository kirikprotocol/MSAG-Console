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
  };
};