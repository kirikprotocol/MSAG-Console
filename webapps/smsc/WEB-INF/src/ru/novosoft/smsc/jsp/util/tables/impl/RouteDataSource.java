/*
 * Author: igork
 * Date: 10.06.2002
 * Time: 17:09:45
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;


public class RouteDataSource extends AbstractDataSourceImpl
{
	private static final String[] columnNames = {"Route ID"};

	public RouteDataSource()
	{
		super(columnNames);
	}

	public void add(Route route)
	{
		super.add(new RouteDataItem(route));
	}

	public void remove(Route route)
	{
		super.remove(new RouteDataItem(route));
	}

	public QueryResultSet query(RouteQuery query_to_run)
	{
		return super.query(query_to_run);
	}
}
