/*
 * Author: igork
 * Date: 13.06.2002
 * Time: 15:44:31
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;


public class SubjectFilter implements Filter
{
  public SubjectFilter()
  {
  }

  public boolean isEmpty()
  {
    return true;
  }

  public boolean isItemAllowed(DataItem item)
  {
    return true;
  }
}
