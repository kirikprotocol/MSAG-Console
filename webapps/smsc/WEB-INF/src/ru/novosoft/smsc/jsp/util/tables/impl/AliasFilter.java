/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 17:14:45
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;


public class AliasFilter implements Filter
{
  public AliasFilter()
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
