/**
 * Created by igork
 * Date: Nov 29, 2002
 * Time: 7:01:26 PM
 */
package ru.novosoft.smsc.jsp.util.tables.impl.category;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.provider.ProviderFilter;

import java.util.Vector;

public class CategoryQuery extends AbstractQueryImpl
{
	public CategoryQuery(int expectedResultsQuantity, CategoryFilter filter, String sortOrder, int startPosition)
	{
		super(expectedResultsQuantity, filter, sortOrder, startPosition);
	}
}
