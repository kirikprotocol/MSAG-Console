package ru.novosoft.smsc.jsp.util.tables.impl.category;

/**
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

public class CategoryQuery extends AbstractQueryImpl
{
  public CategoryQuery(int expectedResultsQuantity, CategoryFilter filter, String sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
