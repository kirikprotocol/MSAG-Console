package ru.novosoft.smsc.jsp.smsview;

import java.util.Date;
import java.util.Vector;
import ru.novosoft.smsc.admin.smsview.SmsQuery;

public class SmsViewFormBean extends SmsQuery
{
  private boolean prevEnabled = false;
  private boolean nextEnabled = true;
  private int rowsTotal = 101;

  public int getStorageType() {
    return storageType;
  };
  public void setStorageType(int type){
    storageType = type;
    System.out.println("setStorageType(): "+storageType);
  };

  public String getFromAddress() {
    return fromAddress;
  };
  public void setFromAddress(String address){
    fromAddress = address;
    System.out.println("setFromAddress(): "+fromAddress);
  };
  public String getToAddress(){
    return toAddress;
  };
  public void setToAddress(String address){
    toAddress = address;
    System.out.println("setToAddress(): "+toAddress);
  };

  public String getFromDate(){
    //return fromDate.toString();
    return "from";
  };
  public void setFromDate(Date date){
    fromDate = date;
  };
  public void setFromDate(String date){
    //fromDate = new Date(date);
    System.out.println("setFromDate(): "+date);
  };
  public String getTillDate(){
    //return tillDate.toString();
    return "till";
  };
  public void setTillDate(Date date){
    tillDate = date;
  };
  public void setTillDate(String date){
    //tillDate = new Date(date);
    System.out.println("setTillDate(): "+date);
  };

  public String getSortBy(){
    return sortBy;
  };
  public void setSortBy(String by){
    sortBy = by;
    System.out.println("setSortBy(): "+sortBy);
  };

  public int getRowsCount(){
    return rowsCount;
  };
  public void setRowsCount(int count){
    rowsCount = count;
    System.out.println("setRowsCount(): "+rowsCount);
  };
  public int getRowIndex(){
    return rowIndex;
  };
  public void setRowIndex(int index){
    rowIndex = index;
    System.out.println("setRowIndex(): "+rowIndex);
  };
  public int getRowsTotal(){
    return rowsTotal;
  };

  public boolean isNextEnabled() {
    return ((rowsCount == -1) ? false : nextEnabled);
  };
  public boolean isPrevEnabled() {
    return ((rowsCount == -1) ? false : prevEnabled);
  };

  public void processQuery() {
    rowIndex = 0;
  }
  public void processPrev() {
    if (rowsCount == -1) rowIndex = 0;
    else rowIndex -= rowsCount;
    if (rowIndex<=0) {
      rowIndex = 0;
      prevEnabled = false;
    }
    if (rowsCount != -1) nextEnabled = true;
  }
  public void processNext() {
    if (rowsCount == -1) rowIndex = 0;
    else rowIndex += rowsCount;
    if (rowIndex>rowsTotal && rowsCount != -1) {
      rowIndex = rowsTotal - rowsCount;
      if (rowIndex<0) rowIndex = 0;
      nextEnabled = false;
    }
    if (rowsCount != -1) prevEnabled = true;
  }

};