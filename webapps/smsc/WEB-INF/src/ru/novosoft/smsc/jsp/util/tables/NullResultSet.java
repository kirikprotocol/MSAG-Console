/*
 * Created by igork
 * Date: 15.11.2002
 * Time: 14:05:46
 */
package ru.novosoft.smsc.jsp.util.tables;

import java.util.Iterator;
import java.util.Vector;

public class NullResultSet implements QueryResultSet
{
	public int size()
	{
		return 0;
	}

	public int getTotalSize()
	{
		return 0;
	}

	public DataItem get(int index)
	{
		return null;
	}

	public Iterator iterator()
	{
		return new Vector().iterator();
	}

	public String[] getColumns()
	{
		return new String[0];
	}

	public Vector getSortOrder()
	{
		return new Vector();
	}

	public boolean isLast()
	{
		return true;
	}

	public void additionalSort(String columnName)
	{
	}
}
