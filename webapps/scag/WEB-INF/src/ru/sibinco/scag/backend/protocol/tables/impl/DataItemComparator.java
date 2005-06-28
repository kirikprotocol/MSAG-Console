package ru.sibinco.scag.backend.protocol.tables.impl;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 20:03:37
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.scag.backend.protocol.tables.DataItem;

import java.util.Comparator;
import java.util.Vector;


class DataItemComparator implements Comparator
{
  private String fieldName = null;

  public DataItemComparator(String fieldNameToCompare)
  {
    fieldName = fieldNameToCompare;
  }

  public int compare(Object o1, Object o2)
  {
    DataItem i1 = (DataItem) o1;
    DataItem i2 = (DataItem) o2;
    Object val1 = i1.getValue(fieldName);
    Object val2 = i2.getValue(fieldName);
    if (val1 instanceof String)
      return ((String) val1).compareToIgnoreCase((String) val2);
    else if (val1 instanceof Vector) {
      Vector v1 = (Vector) val1;
      Vector v2 = (Vector) val2;
      int count = Math.min(v1.size(), v2.size());
      for (int i = 0; i < count; i++) {
        int result = ((String) v1.get(i)).compareToIgnoreCase((String) v2.get(i));
        if (result != 0)
          return result;
      }
      return v1.size() - v2.size();
    }
    else if (val1 instanceof Boolean) {
      if (((Boolean) val1).booleanValue()) {
        return ((Boolean) val2).booleanValue()
                ? 0
                : 1;
      }
      else {
        return ((Boolean) val2).booleanValue()
                ? -1
                : 0;
      }
    }
    else
      return 0;
  }
}
