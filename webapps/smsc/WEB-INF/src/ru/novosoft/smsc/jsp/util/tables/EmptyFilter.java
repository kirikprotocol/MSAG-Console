package ru.novosoft.smsc.jsp.util.tables;


/**
 * Created by igork
 * Date: Aug 18, 2003
 * Time: 3:27:25 PM
 */
public class EmptyFilter implements Filter
{
  public boolean isEmpty()
  {
    return true;
  }

  public boolean isItemAllowed(DataItem item)
  {
    return true;
  }

}
