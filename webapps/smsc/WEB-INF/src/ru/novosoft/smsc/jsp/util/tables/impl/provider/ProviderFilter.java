package ru.novosoft.smsc.jsp.util.tables.impl.provider;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

public class ProviderFilter implements Filter
{
  private String nameFilter = "";


  private boolean isStringEmpty(String s)
  {
    return s == null || s.length() == 0;
  }

  public boolean isEmpty()
  {
    return isStringEmpty(nameFilter);
  }

  public String getNameFilter()
  {
    return nameFilter;
  }

  public void setNameFilter(String nameFilter)
  {
    this.nameFilter = nameFilter;
  }

  public boolean isItemAllowed(DataItem item)
  {
    ProviderDataItem uditem = (ProviderDataItem) item;
    return (isStringEmpty(nameFilter) || uditem.getName().matches(nameFilter));
  }


}
