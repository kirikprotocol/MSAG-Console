package ru.sibinco.scag.backend.protocol.tables;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 19:36:04
 * To change this template use File | Settings | File Templates.
 */


public interface Filter
{
  boolean isEmpty();

  boolean isItemAllowed(DataItem item);
}
