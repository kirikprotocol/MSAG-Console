package ru.sibinco.lib.backend.util;


import org.apache.log4j.Logger;

import java.lang.reflect.Method;
import java.util.Comparator;


/**
 * Created by igork
 * Date: 09.03.2004
 * Time: 15:36:28
 */
public final class SortByPropertyComparator implements Comparator
{
  private Logger logger = Logger.getLogger(this.getClass());
  private final String sort;
  private final boolean negativeSort;

  public SortByPropertyComparator(String sort)
  {
    negativeSort = sort != null && sort.length() > 0 && sort.charAt(0) == '-';
    this.sort = sort == null && sort.length() > 0
                ? "login"
                : negativeSort
                  ? sort.substring(1)
                  : sort;
  }

  public int compare(Object o1, Object o2)
  {
    if (o1 == null && o2 == null)
      return 0;
    if (o1 == null)
      return -1;
    if (o2 == null)
      return 1;

    if (o1.getClass() != o2.getClass())
      return 0;
    String methodName = "get" + Character.toUpperCase(sort.charAt(0)) + sort.substring(1);
    String boolMethodName = "is" + Character.toUpperCase(sort.charAt(0)) + sort.substring(1);
    try {
      Method method = null;
      try {
        method = o1.getClass().getMethod(methodName, new Class[0]);
      } catch (NoSuchMethodException e) {
        method = o1.getClass().getMethod(boolMethodName, new Class[0]);
      }
      if (method.getReturnType() == String.class) {
        String s1 = (String) method.invoke(o1, new Object[0]);
        String s2 = (String) method.invoke(o2, new Object[0]);
        return negativeSort
               ? s2.compareToIgnoreCase(s1)
               : s1.compareToIgnoreCase(s2);
      } else if ((Comparable.class.isAssignableFrom(method.getReturnType()))
                 || (method.getReturnType() == boolean.class)
                 || (method.getReturnType() == int.class)
                 || (method.getReturnType() == long.class)) {
        Comparable c1 = (Comparable) method.invoke(o1, new Object[0]);
        Comparable c2 = (Comparable) method.invoke(o2, new Object[0]);
        return negativeSort
               ? c2.compareTo(c1)
               : c1.compareTo(c2);
      } else
        return 0;
    } catch (Throwable e) {
      logger.debug("Could not compare two objects [\"" + o1 + "\", \"" + o2 + "\"] of type \"" + o1.getClass().getName() + "\" by field \"" + sort + "\"", e);
      return 0;
    }
  }
}
