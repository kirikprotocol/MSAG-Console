package ru.novosoft.smsc.jsp.util.tables.impl.dl;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

/**
 * Created by igork
 * Date: 13.03.2003
 * Time: 22:07:01
 */
public class DlFilter implements Filter
{
	private String[] names = new String[0];

	public boolean isEmpty()
	{
		return names == null || names.length == 0;
	}

	public boolean isItemAllowed(DataItem item)
	{
		if (isEmpty())
			return true;
		for (int i = 0; i < names.length; i++)
		{
			if (((String)item.getValue("name")).matches(".*" + names[i] + ".*"))
				return true;
		}
		return false;
	}

	public String[] getNames()
	{
		return names;
	}

	public void setNames(String[] names)
	{
		this.names = names;
	}
}
