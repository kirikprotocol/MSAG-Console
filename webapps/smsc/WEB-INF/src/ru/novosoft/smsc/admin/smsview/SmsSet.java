package ru.novosoft.smsc.admin.smsview;

/**
 * Title:        SMSC Java Administration
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:      Novosoft
 * @author
 * @version 1.0
 */

import java.util.*;

import ru.novosoft.smsc.admin.smsview.SmsRow;

public class SmsSet
{
  public static int MAX_SMS_FETCH_COUNT = 10000;

  private Vector rows = new Vector();

  public int getRowsCount() {
    return rows.size();
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

	public List getRowsList()
	{
		return rows;
	}
}