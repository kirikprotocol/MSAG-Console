/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 7:02:44 PM
 */
package ru.novosoft.smsc.jsp.util.tables.impl.category;

import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.impl.provider.ProviderDataItem;

public class CategoryFilter implements Filter
{
	private String nameFilter = "";


	private boolean isStringEmpty(String s)
	{
		return s == null || s.length() == 0;
	}

	public boolean isEmpty()
	{
		return isStringEmpty(nameFilter) ;
	}

  public String getNameFilter() {
    return nameFilter;
  }

  public void setNameFilter(String nameFilter) {
    this.nameFilter = nameFilter;
  }

	public boolean isItemAllowed(DataItem item)
	{
		CategoryDataItem uditem = (CategoryDataItem) item;
		return (isStringEmpty(nameFilter) || uditem.getName().matches(nameFilter));
	}


}
