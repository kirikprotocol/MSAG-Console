/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 15:27:04
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.*;
import ru.novosoft.smsc.util.SortedVector;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Vector;


abstract class AbstractDataSourceImpl implements DataSource
{
	protected Vector data = new Vector();
	protected Map sorts = new HashMap();
	protected String[] columnNames = null;

	protected AbstractDataSourceImpl(String[] columnNames)
	{
		this.columnNames = columnNames;
		for (int i = 0; i < columnNames.length; i++)
		{
			sorts.put(columnNames[i], new SortedVector(new DataItemComparator(columnNames[i])));
		}
	}

	public String[] getColumnNames()
	{
		return columnNames;
	}

	protected void add(DataItem item)
	{
		data.add(item);
		for (Iterator i = sorts.values().iterator(); i.hasNext();)
		{
			((SortedVector) i.next()).add(item);
		}
	}

	protected void remove(DataItem item)
	{
		data.remove(item);
		for (Iterator i = sorts.values().iterator(); i.hasNext();)
		{
			((SortedVector) i.next()).remove(item);
		}
	}

	public void clear()
	{
		data.clear();
		for (Iterator i = sorts.values().iterator(); i.hasNext();)
		{
			((SortedVector) i.next()).clear();
		}
	}

	private int findPositionAfterFiltering(Vector dataVector, Filter filter, int position)
	{
		int count = position;
		int i = 0;
		while (i < dataVector.size() && count > 0)
		{
			if (filter.isItemAllowed((DataItem) dataVector.get(i++)))
				count--;
		}
		while (i < dataVector.size() && !filter.isItemAllowed((DataItem) dataVector.get(i)))
			i++;
		return i;
	}

	private int findPositionAfterFilteringBackward(Vector dataVector, Filter filter, int position)
	{
		int count = position;
		int i = dataVector.size() - 1;
		while (i >= 0 && count > 0)
		{
			if (filter.isItemAllowed((DataItem) dataVector.get(i--)))
				count--;
		}
		while (i >= 0 && !filter.isItemAllowed((DataItem) dataVector.get(i)))
			i--;
		return i;
	}

	public QueryResultSet query(Query query_to_run)
	{
		String sort = (String) query_to_run.getSortOrder().get(0);
		boolean isNegativeSort = false;
		if (sort.charAt(0) == '-')
		{
			sort = sort.substring(1);
			isNegativeSort = true;
		}

		Vector srcVector = data;
		if (query_to_run.getSortOrder().size() > 0)
			srcVector = (SortedVector) sorts.get(sort);
		if (query_to_run.getSortOrder().size() > 1)
			throw new UnsupportedOperationException("Hierarchical sorts not implemented");

		QueryResultSetImpl results = new QueryResultSetImpl(columnNames, query_to_run.getSortOrder());

		Filter filter = query_to_run.getFilter();
		int count = query_to_run.getExpectedResultsQuantity();
		for (int i = isNegativeSort
				  ? findPositionAfterFilteringBackward(srcVector, filter, query_to_run.getStartPosition())
				  : findPositionAfterFiltering(srcVector, filter, query_to_run.getStartPosition());
			  i >= 0 && i < srcVector.size() && results.size() < count;
			  i = isNegativeSort ? i - 1 : i + 1)
		{
			DataItem item = (DataItem) srcVector.get(i);
			if (filter.isItemAllowed(item))
				results.add(item);
		}

		return results;
	}
}