/*
 * Author: igork
 * Date: 13.06.2002
 * Time: 15:42:34
 */
package ru.novosoft.smsc.jsp.util.tables.impl.subject;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

import java.util.Vector;


public class SubjectQuery extends AbstractQueryImpl
{
	public SubjectQuery(int expectedResultsQuantity, SubjectFilter filter, Vector sortOrder, int startPosition)
	{
		super(expectedResultsQuantity, filter, sortOrder, startPosition);
	}
}
