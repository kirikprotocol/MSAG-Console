/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:41
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

public class ProfileFilter implements Filter
{
  private boolean empty = true;

  public ProfileFilter()
  {
    empty = true;
  }

  public boolean isEmpty()
  {
    return empty;
  }

  public boolean isItemAllowed(DataItem item)
  {
    return true;
  }
}
