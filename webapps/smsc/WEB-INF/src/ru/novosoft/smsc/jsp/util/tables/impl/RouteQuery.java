/*
 * Author: igork
 * Date: 13.06.2002
 * Time: 16:26:45
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import java.util.Vector;


public class RouteQuery extends AbstractQueryImpl
{
  public RouteQuery(int expectedResultsQuantity, RouteFilter filter, Vector sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
