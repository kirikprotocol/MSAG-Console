/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 7:02:44 PM
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

public class UserFilter implements Filter
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
