package ru.novosoft.smsc.admin.smsview;

/**
 * Title:        SMSC Java Administration
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:      Novosoft
 * @author
 * @version 1.0
 */

import java.util.Date;

public class SmsQuery
{
  public static int SMS_OPERATIVE_STORAGE_TYPE = 100;
  public static int SMS_ARCHIVE_STORAGE_TYPE = 200;

  private int    storageType = SMS_ARCHIVE_STORAGE_TYPE;
  private int    rowsMaximum = 500;
  private String fromAddress = "*";
  private String toAddress = "*";
  private String srcSmeId = "*";
  private String dstSmeId = "*";
  private String routeId = "*";
  private String smsId = "*";
  private Date   fromDate = new Date();
  private Date   tillDate = new Date();
  private String sortBy = "Date";
  private boolean fromDateEnabled = false;
  private boolean tillDateEnabled = true;

  public int getStorageType() { return storageType; }
  public void setStorageType(int type) { storageType = type; }
  public int getRowsMaximum() { return rowsMaximum; }
  public void setRowsMaximum(int max) { rowsMaximum = max; }

  public String getFromAddress() { return fromAddress; }
  public void setFromAddress(String address) { fromAddress = address; }
  public String getToAddress() { return toAddress; }
  public void setToAddress(String address) { toAddress = address; }

  public String getSrcSmeId() { return srcSmeId; }
  public void setSrcSmeId(String id) { srcSmeId = id; }
  public String getDstSmeId() { return dstSmeId; }
  public void setDstSmeId(String id) { dstSmeId = id; }
  public String getRouteId() { return routeId; }
  public void setRouteId(String id) { routeId = id; }
  public String getSmsId() { return smsId; }
  public void setSmsId(String id) { smsId = id; }

  public void setFromDateEnabled(boolean enabled) { fromDateEnabled = enabled; }
  public boolean getFromDateEnabled() { return fromDateEnabled; }
  public Date getFromDate() { return fromDate; }
  public void setFromDate(Date date) { fromDate = date; }

  public void setTillDateEnabled(boolean enabled) { tillDateEnabled = enabled; }
  public boolean getTillDateEnabled() { return tillDateEnabled; }
  public Date getTillDate() { return tillDate; }
  public void setTillDate(Date date) { tillDate = date; }

  public String getSortBy() { return sortBy; }
  public void setSortBy(String by) { sortBy = by; }

};