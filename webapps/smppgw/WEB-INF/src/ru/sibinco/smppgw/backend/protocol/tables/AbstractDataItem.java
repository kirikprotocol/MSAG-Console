package ru.sibinco.smppgw.backend.protocol.tables;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 19:54:40
 * To change this template use File | Settings | File Templates.
 */

import java.util.HashMap;
import java.util.Map;


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
}
