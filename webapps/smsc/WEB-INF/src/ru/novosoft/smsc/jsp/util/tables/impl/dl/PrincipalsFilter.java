package ru.novosoft.smsc.jsp.util.tables.impl.dl;

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 23.09.2003
 * Time: 16:53:21
 */
public class PrincipalsFilter implements Filter
{
  private String filterAddress;

  public PrincipalsFilter(String filterAddress)
  {
    this.filterAddress = filterAddress;
  }

  public boolean isEmpty()
  {
    return filterAddress == null || filterAddress.length() == 0 || filterAddress.equals("*") || filterAddress.equals("%");
  }

  public boolean isItemAllowed(DataItem item)
  {
    // �������� �������� ��� ���������� ������� � PrincipalsDataSource
    return true;
  }

  public String getFilterAddress()
  {
    return filterAddress;
  }

  public void setFilterAddress(String filterAddress)
  {
    this.filterAddress = filterAddress;
  }
}
