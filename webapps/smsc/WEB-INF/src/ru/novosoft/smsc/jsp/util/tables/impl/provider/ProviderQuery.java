/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 7:01:26 PM
 */
package ru.novosoft.smsc.jsp.util.tables.impl.provider;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

import java.util.Vector;

public class ProviderQuery extends AbstractQueryImpl
{
	public ProviderQuery(int expectedResultsQuantity, ProviderFilter filter, String sortOrder, int startPosition)
	{
		super(expectedResultsQuantity, filter, sortOrder, startPosition);
	}
}
