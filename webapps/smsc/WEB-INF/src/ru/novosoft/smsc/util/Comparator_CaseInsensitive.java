package ru.novosoft.smsc.util;

import java.util.Comparator;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 6:16:34 PM
 */
public class Comparator_CaseInsensitive implements Comparator {
  public int compare(Object o1, Object o2)
  {
    if (o1 instanceof String && o2 instanceof String)
    {
      String s1 = (String) o1;
      String s2 = (String) o2;
      return s1.compareToIgnoreCase(s2);
    }
    if (o1 instanceof Comparable)
      return ((Comparable)o1).compareTo(o2);
    if (o2 instanceof Comparable)
      return -((Comparable)o2).compareTo(o1);
    return 0;
  }
}
