package ru.sibinco.scag.backend.protocol.tables;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 19:35:11
 * To change this template use File | Settings | File Templates.
 */

import java.util.Vector;


public interface Query
{
  Filter getFilter();

  int getStartPosition();

  int getExpectedResultsQuantity();

  Vector getSortOrder();
}
