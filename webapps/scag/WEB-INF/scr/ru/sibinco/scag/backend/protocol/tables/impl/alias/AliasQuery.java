package ru.sibinco.scag.backend.protocol.tables.impl.alias;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 19:31:20
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.scag.backend.protocol.tables.impl.AbstractQueryImpl;

import java.util.Vector;


public class AliasQuery extends AbstractQueryImpl
{
  public AliasQuery(int expectedResultsQuantity, AliasFilter filter, Vector sortOrder, int startPosition)
  {
    super(expectedResultsQuantity, filter, sortOrder, startPosition);
  }
}