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

  public static short INDEX_PAGES_LIST_SIZE = 5;

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
      view.setSmsc(context.getSmsc());
    }
  }
  public AppContext getAppContext() {
    return context;
  }

  public void processQuery()
  {
    getDates();
    rows = view.getSmsSet(this);
    processFirstIndexing();
    setDates();
  }
  public void processDeleteAll()
  {
    if (getStorageType() == SmsQuery.SMS_ARCHIVE_STORAGE_TYPE) {
      deletedRowsCount = view.delArchiveSmsSet(rows);
      getStorageType();
    }
    else {
      deletedRowsCount = view.delOperativeSmsSet(rows);
      rows = null;
      processFirstIndexing();
    }
  }

  public void moveToPrev()  { processPrevIndexing();  }
  public void moveToNext()  { processNextIndexing();  }
  public void moveToFirst() { processFirstIndexing(); }
  public void moveToLast()  { processLastIndexing();  }

  public void moveToPage(String page) {
    try { processPageIndexing(Integer.parseInt(page)); }
    catch (Exception exc) {};
  }
  public int getRowsCount() {
    return ((rows != null) ? rows.getRowsCount():0);
  }
  public SmsRow getRow(int index){
    return ((rows != null) ? rows.getRow(index):null);
  }
  public int getPagesCount() {
    if (rowsToDisplay < 0) return 0;
    int rowsCount = getRowsCount();
    if (rowsCount < rowsToDisplay) return 1;
    return rowsCount/rowsToDisplay+((rowsCount%rowsToDisplay > 0) ? 1:0);
  }
  public int getPageIndex() {
    return ((rowsToDisplay > 0) ? rowIndex/rowsToDisplay : 0);
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

  public int getDeletedRowsCount() { return deletedRowsCount; }

  /* --------------------------- Private Part ----------------------------- */

  private boolean prevEnabled = false;
  private boolean nextEnabled = true;

  private int deletedRowsCount = 0;
  private int rowsToDisplay = 10;
  private int rowIndex = 0;

  private int fromDateYear, fromDateMonth, fromDateDay, fromDateHour, fromDateMinute;
  private int toDateYear, toDateMonth, toDateDay, toDateHour, toDateMinute;

  private void processFirstIndexing() {
    rowIndex = 0; prevEnabled = false;
    if (rowsToDisplay == -1) {
      nextEnabled = false;
    } else if (getRowsCount() > rowsToDisplay) {
      nextEnabled = true;
    }
  }
  private void processLastIndexing() {
    if (rowsToDisplay == -1) {
      rowIndex = 0; prevEnabled = false; nextEnabled = false;
    } else {
      prevEnabled = true; nextEnabled = false;
      int restRows = getRowsCount()%rowsToDisplay;
      rowIndex = getRowsCount()-((restRows>0) ? restRows:rowsToDisplay);
      if (rowIndex<0) rowIndex = 0;
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
  private void processPageIndexing(int page)
  {
    int pagesCount = getPagesCount();
    if (page<0 || page>pagesCount-1 || rowsToDisplay<0) return;
    rowIndex = page*rowsToDisplay;
    prevEnabled=true; nextEnabled=true;
    if (page == 0) prevEnabled=false;
    if (page >= pagesCount-1) nextEnabled=false;
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
    GregorianCalendar tdc = new GregorianCalendar();
    tdc.setTime(getTillDate());
    toDateYear = tdc.get(GregorianCalendar.YEAR);
    toDateMonth = tdc.get(GregorianCalendar.MONTH);
    toDateDay = tdc.get(GregorianCalendar.DAY_OF_MONTH);
    toDateHour = tdc.get(GregorianCalendar.HOUR_OF_DAY);
    toDateMinute = tdc.get(GregorianCalendar.MINUTE);
  }
  private void getDates()
  {
    setFromDate((new GregorianCalendar(
                      fromDateYear, fromDateMonth, fromDateDay,
                      fromDateHour, fromDateMinute, 0)).getTime());
    setTillDate((new GregorianCalendar(
                      toDateYear, toDateMonth, toDateDay,
                      toDateHour, toDateMinute, 0)).getTime());
  }

};