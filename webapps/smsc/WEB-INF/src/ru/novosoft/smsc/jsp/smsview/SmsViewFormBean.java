package ru.novosoft.smsc.jsp.smsview;

import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Vector;

import ru.novosoft.util.jsp.AppContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import ru.novosoft.smsc.admin.smsview.SmsQuery;
import ru.novosoft.smsc.admin.smsview.SmsView;
import ru.novosoft.smsc.admin.smsview.SmsSet;
import ru.novosoft.smsc.admin.smsview.SmsRow;

public class SmsViewFormBean extends SmsQuery
{
  public static int START_YEAR_COUNTER  = 2002;
  public static int FINISH_YEAR_COUNTER = START_YEAR_COUNTER+30;

  private SmsSet  rows = null;
  private SmsView view = new SmsView();
  private SMSCAppContext  context = null;

  public static String monthesNames[] = {
    "January", "February", "March", "April",
    "May", "June", "July", "August", "September",
    "October", "November", "December"
  };

  public SmsViewFormBean() {
    setDates();
    setFromDateEnabled(false);
    setTillDateEnabled(true);
  }

  public void setAppContext(Object ctx) {
    if (context == null && ctx instanceof SMSCAppContext) {
      context = (SMSCAppContext)ctx;
      view.setDataSource(context.getConnectionPool());
    }
  }
  public AppContext getAppContext() {
    return context;
  }

  public void processQuery()
  {
    getDates();
    rows = view.getSmsSet(this);
    processQueryIndexing();
    setDates();
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

  public int getRowsToDisplay() { return rowsToDisplay; }
  public void setRowsToDisplay(int count) { rowsToDisplay = count; }
  public int getRowIndex() { return rowIndex; }

  public boolean isNextEnabled() {
    return ((rowsToDisplay == -1) ? false : nextEnabled);
  }
  public boolean isPrevEnabled() {
    return ((rowsToDisplay == -1) ? false : prevEnabled);
  }

  public void setFromDateDay(int day) { fromDateDay = day; }
  public int getFromDateDay() { return fromDateDay; }
  public void setFromDateMonth(int month) { fromDateMonth = month; }
  public int getFromDateMonth() { return fromDateMonth; }
  public void setFromDateYear(int year) { fromDateYear = year; }
  public int getFromDateYear() { return fromDateYear; }
  public void setFromDateHour(int hour) { fromDateHour = hour; }
  public int getFromDateHour() { return fromDateHour; }
  public void setFromDateMinute(int minute) { fromDateMinute = minute; }
  public int getFromDateMinute() { return fromDateMinute; }
  public void setFromDateSecond(int second) { fromDateSecond = second; }
  public int getFromDateSecond() { return fromDateSecond; }

  public void setToDateDay(int day) { toDateDay = day; }
  public int getToDateDay() { return toDateDay; }
  public void setToDateMonth(int month) { toDateMonth = month; }
  public int getToDateMonth() { return toDateMonth; }
  public void setToDateYear(int year) { toDateYear = year; }
  public int getToDateYear() { return toDateYear; }
  public void setToDateHour(int hour) { toDateHour = hour; }
  public int getToDateHour() { return toDateHour; }
  public void setToDateMinute(int minute) { toDateMinute = minute; }
  public int getToDateMinute() { return toDateMinute; }
  public void setToDateSecond(int second) { toDateSecond = second; }
  public int getToDateSecond() { return toDateSecond; }

  /* --------------------------- Private Part ----------------------------- */

  private boolean prevEnabled = false;
  private boolean nextEnabled = true;

  private int rowsToDisplay = 10;
  private int rowIndex = 0;

  private int fromDateYear, fromDateMonth, fromDateDay;
  private int fromDateHour, fromDateMinute, fromDateSecond;
  private int toDateYear, toDateMonth, toDateDay;
  private int toDateHour, toDateMinute, toDateSecond;

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

  private void setDates()
  {
    GregorianCalendar fdc = new GregorianCalendar();
    fdc.setTime(getFromDate());
    fromDateYear = fdc.get(GregorianCalendar.YEAR);
    fromDateMonth = fdc.get(GregorianCalendar.MONTH);
    fromDateDay = fdc.get(GregorianCalendar.DAY_OF_MONTH);
    fromDateHour = fdc.get(GregorianCalendar.HOUR_OF_DAY);
    fromDateMinute = fdc.get(GregorianCalendar.MINUTE);
    fromDateSecond = fdc.get(GregorianCalendar.SECOND);
    GregorianCalendar tdc = new GregorianCalendar();
    tdc.setTime(getTillDate());
    toDateYear = tdc.get(GregorianCalendar.YEAR);
    toDateMonth = tdc.get(GregorianCalendar.MONTH);
    toDateDay = tdc.get(GregorianCalendar.DAY_OF_MONTH);
    toDateHour = tdc.get(GregorianCalendar.HOUR_OF_DAY);
    toDateMinute = tdc.get(GregorianCalendar.MINUTE);
    toDateSecond = tdc.get(GregorianCalendar.SECOND);
  }
  private void getDates()
  {
    setFromDate((new GregorianCalendar(
                      fromDateYear, fromDateMonth, fromDateDay,
                      fromDateHour, fromDateMinute, fromDateSecond)).getTime());
    setTillDate((new GregorianCalendar(
                      toDateYear, toDateMonth, toDateDay,
                      toDateHour, toDateMinute, toDateSecond)).getTime());
  }

};