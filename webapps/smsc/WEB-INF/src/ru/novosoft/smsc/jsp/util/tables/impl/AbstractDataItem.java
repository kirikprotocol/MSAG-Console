/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 17:01:37
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.HashMap;
import java.util.Map;


abstract class AbstractDataItem implements DataItem
{
  protected Map values = new HashMap();

  public Object getValue(String fieldName)
  {
    return values.get(fieldName);
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof AbstractDataItem)
      return values.equals(((AbstractDataItem) obj).values);
    else
      return false;
  }
}
