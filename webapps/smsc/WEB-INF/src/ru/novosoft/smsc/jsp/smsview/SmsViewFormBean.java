package ru.novosoft.smsc.jsp.smsview;

import java.util.Date;
import java.util.Vector;

import ru.novosoft.util.jsp.AppContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import ru.novosoft.smsc.admin.smsview.SmsQuery;
import ru.novosoft.smsc.admin.smsview.SmsView;
import ru.novosoft.smsc.admin.smsview.SmsSet;
import ru.novosoft.smsc.admin.smsview.SmsRow;

public class SmsViewFormBean extends SmsQuery
{
  private SmsSet  rows = null;
  private SmsView view = new SmsView();
  private SMSCAppContext  context = null;

  private int fromDateDay = 1;
  private int fromDateMonth = 0;
  private int fromDateYear = 2002;
  private int fromDateHour = 0;
  private int fromDateMinute = 0;
  private int fromDateSecond = 0;

  public String monthesNames[] = {
    "January", "February", "March", "April",
    "May", "June", "July", "August", "September",
    "October", "November", "December"
  };

  public void setAppContext(Object ctx) {
    if (context == null && ctx instanceof SMSCAppContext) {
      context = (SMSCAppContext)ctx;
      view.setDataSource(context.getConnectionPool());
    }
  }
  public AppContext getAppContext() {
    return context;
  }

  public void setFromDateDay(int day) {
    fromDateDay = day;
  }
  public int getFromDateDay() {
    return fromDateDay;
  }
  public void setFromDateMonth(int month) {
    fromDateMonth = month;
  }
  public int getFromDateMonth() {
    return fromDateMonth;
  }
  public void setFromDateYear(int year) {
    fromDateYear = year;
  }
  public int getFromDateYear() {
    return fromDateYear;
  }
  public void setFromDateHour(int hour) {
    fromDateHour = hour;
  }
  public int getFromDateHour() {
    return fromDateHour;
  }
  public void setFromDateMinute(int minute) {
    fromDateMinute = minute;
  }
  public int getFromDateMinute() {
    return fromDateMinute;
  }
  public void setFromDateSecond(int second) {
    fromDateSecond = second;
  }
  public int getFromDateSecond() {
    return fromDateSecond;
  }

  public void processQuery() {
    rows = view.getSmsSet(this);
    processQueryIndexing();
  }
  public void processPrev() {
    processPrevIndexing();
  }
  public void processNext() {
    processNextIndexing();
  }
  public int getRowsCount() {
    return ((rows != null) ? rows.getRowsCount():0);
  }
  public SmsRow getRow(int index){
    return ((rows != null) ? rows.getRow(index):null);
  }

  public int getRowsToDisplay(){
    return rowsToDisplay;
  }
  public void setRowsToDisplay(int count){
    rowsToDisplay = count;
  }
  public int getRowIndex(){
    return rowIndex;
  }

  public boolean isNextEnabled() {
    return ((rowsToDisplay == -1) ? false : nextEnabled);
  }
  public boolean isPrevEnabled() {
    return ((rowsToDisplay == -1) ? false : prevEnabled);
  }

  /* --------------------------- Private Part ----------------------------- */

  private boolean prevEnabled = false;
  private boolean nextEnabled = true;

  private int rowsToDisplay = 10;
  private int rowIndex = 0;

  private void processQueryIndexing() {
    rowIndex = 0; prevEnabled = false;
    if (rowsToDisplay == -1) {
      nextEnabled = false;
    } else if (getRowsCount() > rowsToDisplay) {
      nextEnabled = true;
    }
  }
  private void processPrevIndexing() {
    if (rowsToDisplay == -1) rowIndex = 0;
    else rowIndex -= rowsToDisplay;
    if (rowIndex<=0) {
      rowIndex = 0;
      prevEnabled = false;
    }
    if (rowsToDisplay != -1) nextEnabled = true;
  }
  private void processNextIndexing() {
    if (rowsToDisplay == -1) {
      rowIndex = 0; prevEnabled = false; nextEnabled = false;
    } else {
      prevEnabled = true; nextEnabled = true;
      rowIndex += rowsToDisplay;
      if (rowIndex+rowsToDisplay >= getRowsCount()) {
        if (rowIndex >= getRowsCount()) {
          rowIndex -= rowsToDisplay;
          if (rowIndex<0) rowIndex = 0;
        }
        nextEnabled = false;
      }
    }
  }

};