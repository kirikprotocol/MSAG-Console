package ru.novosoft.smsc.admin.smsview;

/**
 * Title:        SMSC Java Administration
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:      Novosoft
 * @author
 * @version 1.0
 */

import java.util.Vector;
import java.util.Date;

public class SmsQuery
{
  public static int SMS_OPERATIVE_STORAGE_TYPE = 100;
  public static int SMS_ARCHIVE_STORAGE_TYPE = 200;

  private int    storageType = SMS_ARCHIVE_STORAGE_TYPE;
  private String fromAddress = "***";
  private String toAddress = "***";
  private Date   fromDate = new Date();
  private Date   tillDate = new Date();
  private String sortBy = "Date";
  private boolean fromDateEnabled = false;
  private boolean tillDateEnabled = false;

  public int getStorageType() {
    return storageType;
  };
  public void setStorageType(int type){
    storageType = type;
  };
  public String getFromAddress() {
    return fromAddress;
  };
  public void setFromAddress(String address){
    fromAddress = address;
  };
  public String getToAddress(){
    return toAddress;
  };
  public void setToAddress(String address){
    toAddress = address;
  };

  public void setFromDateEnabled(boolean enabled) {
    fromDateEnabled = enabled;
  }
  public boolean getFromDateEnabled() {
    return fromDateEnabled;
  }
  public String getFromDate() {
    return fromDate.toString();
  };
  public void setFromDate(Date date) {
    fromDate = date;
  };
  public void setFromDate(String date) {
    //fromDate = new Date(date);
  };
  public void setTillDateEnabled(boolean enabled) {
    tillDateEnabled = enabled;
  }
  public boolean getTillDateEnabled() {
    return tillDateEnabled;
  }
  public String getTillDate(){
    return tillDate.toString();
  };
  public void setTillDate(Date date){
    tillDate = date;
  };
  public void setTillDate(String date){
    //tillDate = new Date(date);
  };
  public String getSortBy(){
    return sortBy;
  };
  public void setSortBy(String by){
    sortBy = by;
  };

};