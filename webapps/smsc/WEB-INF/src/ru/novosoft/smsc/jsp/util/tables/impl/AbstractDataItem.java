package ru.novosoft.smsc.jsp.util.tables.impl;

/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 17:01:37
 */

import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;


public abstract class AbstractDataItem implements DataItem
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

  public String toString() {
    StringBuffer buffer= new StringBuffer();
    Iterator iter = values.entrySet().iterator();
    while(iter.hasNext()) {
      Map.Entry e = (Map.Entry)iter.next();
      buffer.append("{").append(e.getKey().toString()).append("=>").append(e.getValue().toString()).append("}");
    }
    return buffer.toString();
  }

}
