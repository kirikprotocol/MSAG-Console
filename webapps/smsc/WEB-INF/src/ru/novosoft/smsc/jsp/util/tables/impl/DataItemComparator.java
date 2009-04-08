package ru.novosoft.smsc.jsp.util.tables.impl;

/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 16:04:01
 */

import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.Comparator;
import java.util.Vector;


class DataItemComparator implements Comparator
{
  private String fieldName = null;
  private boolean reverse;

  public DataItemComparator(String fieldNameToCompare)
  {
    this(fieldNameToCompare, false);
  }

  public DataItemComparator(String fieldName, boolean reverse) {
    this.fieldName = fieldName;
    this.reverse = reverse;
  }

  public int compare(Object o1, Object o2) {
    if (reverse)
      return -_compare(o1, o2);
    else
      return _compare(o1, o2);
  }

  public int _compare(Object o1, Object o2)
  {
    DataItem i1 = (DataItem) o1;
    DataItem i2 = (DataItem) o2;
    Object val1 = i1.getValue(fieldName);
    Object val2 = i2.getValue(fieldName);

    if (val1 == null && val2 != null)
      return -1;
    else if (val2 == null && val1 != null)
      return 1;
    else if (val1 == val2)
      return 0;

    if (val1 instanceof String) {
      return ((String) val1).compareToIgnoreCase((String) val2);

    } else if (val1 instanceof Vector) {
      Vector v1 = (Vector) val1;
      Vector v2 = (Vector) val2;
      int count = Math.min(v1.size(), v2.size());
      for (int i = 0; i < count; i++) {
        int result = ((String) v1.get(i)).compareToIgnoreCase((String) v2.get(i));
        if (result != 0)
          return result;
      }
      return v1.size() - v2.size();

    } else if (val1 instanceof Boolean) {
      if (((Boolean) val1).booleanValue())
        return ((Boolean) val2).booleanValue() ? 0 : 1;
      else
        return ((Boolean) val2).booleanValue() ? -1 : 0;

    } else if (val1 instanceof Comparable) {

      return ((Comparable)val1).compareTo(val2);

    } else
      return 0;
  }
}
