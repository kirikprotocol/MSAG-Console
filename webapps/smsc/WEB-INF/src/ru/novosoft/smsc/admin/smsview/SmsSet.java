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
import java.util.Enumeration;

import ru.novosoft.smsc.admin.smsview.SmsRow;

public class SmsSet
{
  private Vector rows = new Vector();

  public int getRowsCount() {
    //return rows.size();
    return 101;
  }

  public Enumeration getRows() {
    return rows.elements();
  }
  public SmsRow getRow(int index) {
    return ((SmsRow)rows.elementAt(index));
  }
  public void addRow(SmsRow row) {
    rows.addElement(row);
  }
  public void clean() {
    rows.removeAllElements();
  }
}