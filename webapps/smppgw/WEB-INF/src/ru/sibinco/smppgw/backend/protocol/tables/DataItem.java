package ru.sibinco.smppgw.backend.protocol.tables;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 19:52:27
 * To change this template use File | Settings | File Templates.
 */

public interface DataItem
{
  Object getValue(String fieldName);
}

