package ru.novosoft.smsc.admin.smsview;

/**
 * Title:        SMSC Java Administration
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:      Novosoft
 * @author
 * @version 1.0
 */

import java.util.Collection;
import java.util.Enumeration;
import java.util.List;
import java.util.Vector;

public class SmsSet
{
  public static int MAX_SMS_FETCH_COUNT = 10000;

  private Vector rows = new Vector();
  private boolean hasMore = false;
  private int maxRows = 0;
  private int smesRows = 0;

  public int getRowsCount()
  {
    return rows.size();
  }

  public int getSmesRows()
  {
    return smesRows;
  }

  public void setSmesRows(int smesRows)
  {
    this.smesRows = smesRows;
  }

  public Enumeration getRows()
  {
    return rows.elements();
  }

  public List getRowsList()
  {
    return rows;
  }

  public void addRow(SmsRow row)
  {
    rows.addElement(row);
  }

  public void addAll(Collection smes)
  {
    rows.addAll(smes);
  }

  public void removeRow(SmsRow row)
  {
    rows.removeElement(row);
  }

  public void clean()
  {
    rows.removeAllElements();
    hasMore = false;
  }

  public SmsRow getRow(int index)
  {
    return ((SmsRow) rows.elementAt(index));
  }

  public SmsRow getRow(String id)
  {
    SmsRow pattern = new SmsRow();
    pattern.setId(Long.parseLong(id));
    int index = rows.indexOf(pattern);
    return (index >= 0) ? getRow(index) : null;
  }

  public SmsRow getRow(long id)
  {
    SmsRow pattern = new SmsRow();
    pattern.setId(id);
    int index = rows.indexOf(pattern);
    return (index >= 0) ? getRow(index) : null;
  }

  public boolean isHasMore()
  {
    return (rows.size() > 0) ? hasMore : false;
  }

  public void setHasMore(boolean hasMore)
  {
    this.hasMore = hasMore;
  }

  public int getMaxRows()
  {
    return maxRows;
  }

  public void setMaxRows(int maxRows)
  {
    this.maxRows = maxRows;
  }
}