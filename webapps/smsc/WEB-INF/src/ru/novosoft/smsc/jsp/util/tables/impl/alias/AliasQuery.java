/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 17:07:44
 */
package ru.novosoft.smsc.jsp.util.tables.impl.alias;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractQueryImpl;

import java.util.Vector;


public class AliasQuery extends AbstractQueryImpl
{
  public AliasQuery(int expectedResultsQuantity, AliasFilter filter, Vector sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
