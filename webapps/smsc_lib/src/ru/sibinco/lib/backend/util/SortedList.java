package ru.sibinco.lib.backend.util;

import java.util.*;


/**
 * Created by igork
 * Date: Jan 20, 2003
 * Time: 7:33:12 PM
 */
public class SortedList extends LinkedList
{
  private Comparator comparator;

  public SortedList(Comparator comparator)
  {
    this.comparator = comparator;
  }

  public SortedList(Collection c, Comparator comparator)
  {
    this(comparator);
    addAll(c);
  }

  public SortedList()
  {
    this(new Comparator_CaseInsensitive());
  }

  public SortedList(Collection c)
  {
    this();
    addAll(c);
  }

  public boolean add(Object o)
  {
    int pos = Collections.binarySearch(this, o, comparator);
    if (pos < 0)
      pos = -pos - 1;
    super.add(pos, o);
    return true;
  }

  public boolean addAll(Collection c)
  {
    boolean result = false;
    for (Iterator i = c.iterator(); i.hasNext();)
      result |= add(i.next());
    return result;
  }

  public boolean addAll(int index, Collection c)
  {
    throw new UnsupportedOperationException("Inserting in any specified position is not supported by Sorted list");
  }

  public Object set(int index, Object element)
  {
    throw new UnsupportedOperationException("Setting element in any specified position is not supported by Sorted list");
  }

  public void add(int index, Object element)
  {
    throw new UnsupportedOperationException("Inserting in any specified position is not supported by Sorted list");
  }
}
