package ru.novosoft.smsc.jsp.util.tables;

/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 15:23:10
 */


public interface Filter
{
  boolean isEmpty();

  boolean isItemAllowed(DataItem item);
}
