/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 16:59:45
 */
package ru.novosoft.smsc.jsp.util.tables.impl.alias;

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;


public class AliasDataSource extends AbstractDataSourceImpl
{
	private static final String aliasColumnName = "Alias";
	private static final String hideColumnName = "Hide";
	private static final String[] columnNames = {aliasColumnName, "Address", hideColumnName};

	public AliasDataSource()
	{
		super(columnNames);
	}

	public void add(Alias alias)
	{
		super.add(new AliasDataItem(alias));
	}

	public void remove(Alias alias)
	{
		super.remove(new AliasDataItem(alias));
	}

	public QueryResultSet query(AliasQuery query_to_run)
	{
		QueryResultSet result = super.query(query_to_run);
		final String sortOrder = (String) query_to_run.getSortOrder().get(0);
		if (sortOrder != null && sortOrder.endsWith(hideColumnName))
		{
			result.additionalSort(aliasColumnName);
		}
		return result;
	}
}
