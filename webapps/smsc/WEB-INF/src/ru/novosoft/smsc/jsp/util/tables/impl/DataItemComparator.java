/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 16:04:01
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.Comparator;
import java.util.Vector;


class DataItemComparator implements Comparator
{
	private String fieldName = null;

	public DataItemComparator(String fieldNameToCompare)
	{
		fieldName = fieldNameToCompare;
	}

	public int compare(Object o1, Object o2)
	{
		DataItem i1 = (DataItem) o1;
		DataItem i2 = (DataItem) o2;
		Object val1 = i1.getValue(fieldName);
		Object val2 = i2.getValue(fieldName);
		if (val1 instanceof String)
			return ((String) val1).compareTo((String) val2);
		else if (val1 instanceof Vector)
		{
			Vector v1 = (Vector) val1;
			Vector v2 = (Vector) val2;
			int count = Math.min(v1.size(), v2.size());
			for (int i = 0; i < count; i++)
			{
				int result = ((String) v1.get(i)).compareTo((String) v2.get(i));
				if (result != 0)
					return result;
			}
			return v1.size() - v2.size();
		}
		else if (val1 instanceof Boolean)
		{
			if (((Boolean) val1).booleanValue())
			{
				return ((Boolean) val2).booleanValue()
						  ? 0
						  : 1;
			}
			else
			{
				return ((Boolean) val2).booleanValue()
						  ? -1
						  : 0;
			}
		}
		else
			return 0;
	}
}
