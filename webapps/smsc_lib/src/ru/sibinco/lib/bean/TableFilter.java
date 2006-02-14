package ru.sibinco.lib.bean;

import ru.sibinco.lib.backend.util.SortedList;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 23.01.2006
 * Time: 17:58:47
 * To change this template use File | Settings | File Templates.
 */
public interface TableFilter
{
  SortedList filter(SortedList list);    
}
