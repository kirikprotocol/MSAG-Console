/*
 * Author: igork
 * Date: 13.06.2002
 * Time: 16:27:35
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;


public class RouteFilter implements Filter
{
  public RouteFilter()
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
