/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 7:01:26 PM
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.Vector;

public class UserQuery extends AbstractQueryImpl
{
	public UserQuery(int expectedResultsQuantity, UserFilter filter, Vector sortOrder, int startPosition)
	{
		super(expectedResultsQuantity, filter, sortOrder, startPosition);
	}
}
