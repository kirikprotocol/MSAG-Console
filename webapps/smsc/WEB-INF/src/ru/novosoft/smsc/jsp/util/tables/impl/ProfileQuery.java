/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 18:23:53
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import java.util.Vector;

public class ProfileQuery extends AbstractQueryImpl
{
  public ProfileQuery(int expectedResultsQuantity, ProfileFilter filter, Vector sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}
