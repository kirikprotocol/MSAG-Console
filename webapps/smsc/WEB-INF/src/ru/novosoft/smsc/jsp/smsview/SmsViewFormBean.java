package ru.novosoft.smsc.jsp.smsview;

import java.util.Date;
import java.util.Vector;
import ru.novosoft.util.jsp.AppContext;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import ru.novosoft.smsc.admin.smsview.SmsQuery;

public class SmsViewFormBean extends SmsQuery
{
  private boolean prevEnabled = false;
  private boolean nextEnabled = true;

  private int rowsToDisplay = 10;
  private int rowIndex = 0;

  private SMSCAppContext  context = null;

  public void setAppContext(Object ctx) {
    if (ctx instanceof SMSCAppContext) context = (SMSCAppContext)ctx;
  }
  public AppContext getAppContext() {
    return context;
  }

  public int getRowsToDisplay(){
    return rowsToDisplay;
  };
  public void setRowsToDisplay(int count){
    rowsToDisplay = count;
  };
  public int getRowIndex(){
    return rowIndex;
  };

  public boolean isNextEnabled() {
    return ((rowsToDisplay == -1) ? false : nextEnabled);
  };
  public boolean isPrevEnabled() {
    return ((rowsToDisplay == -1) ? false : prevEnabled);
  };

  public void processQuery() {
    rowIndex = 0; prevEnabled = false;
    if (rowsToDisplay == -1) {
      nextEnabled = false;
    } else if (getRowsCount() > rowsToDisplay) {
      nextEnabled = true;
    }
  };
  public void processPrev() {
    if (rowsToDisplay == -1) rowIndex = 0;
    else rowIndex -= rowsToDisplay;
    if (rowIndex<=0) {
      rowIndex = 0;
      prevEnabled = false;
    }
    if (rowsToDisplay != -1) nextEnabled = true;
  }
  public void processNext() {
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