package ru.novosoft.smsc.jsp.util.tables.impl.mscman;

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * Created by igork
 * Date: Aug 18, 2003
 * Time: 5:03:29 PM
 */
public class MscFilter implements Filter
{
  private String prefix;

  public MscFilter(String prefix)
  {
    this.prefix = prefix;
  }

  public boolean isEmpty()
  {
    return prefix == null || prefix.length() == 0;
  }

  public boolean isItemAllowed(DataItem item)
  {
    if (isEmpty())
      return true;
    else {
      MscDataItem mscItem = (MscDataItem) item;
      return mscItem.getCommutator().startsWith(prefix);
    }
  }

  public String getPrefix()
  {
    return prefix;
  }
}
