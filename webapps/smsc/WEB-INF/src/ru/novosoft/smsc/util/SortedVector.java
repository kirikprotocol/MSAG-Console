/*
 * Author: igork
 * Date: 10.06.2002
 * Time: 15:10:13
 */
package ru.novosoft.smsc.util;

import java.util.*;


public class SortedVector extends Vector
{
  private Comparator comparator = null;

  public SortedVector(Comparator comp)
  {
    comparator = comp;
  }

  public SortedVector(Collection c, Comparator comp)
  {
    super(c);
    comparator = comp;
    Collections.sort(this, comparator);
  }

  public SortedVector(int initialCapacity, Comparator comp)
  {
    super(initialCapacity);
    comparator = comp;
  }

  public SortedVector(int initialCapacity, int capacityIncrement, Comparator comp)
  {
    super(initialCapacity, capacityIncrement);
    comparator = comp;
  }

  public void add(int index, Object element)
  {
    throw new UnsupportedOperationException("Inserting in any specified position is not allowed in SortedVector");
  }

  public synchronized boolean add(Object o)
  {
    int pos = Collections.binarySearch(this, o, comparator);
    if (pos < 0)
      pos = -pos - 1;
    super.add(pos, o);
    return true;
  }

  public synchronized void addElement(Object obj)
  {
    this.add(obj);
  }

  public synchronized boolean addAll(Collection c)
  {
    for (Iterator i = c.iterator(); i.hasNext();)
    {
      this.add(i.next());
    }
    return !c.isEmpty();
  }

  public synchronized boolean addAll(int index, Collection c)
  {
    throw new UnsupportedOperationException("Inserting in any specified position is not allowed in SortedVector");
  }

  /*public synchronized void insertElementAt(Object obj, int index)
  {
    this.add(index, obj);
  }*/

  public synchronized Object set(int index, Object element)
  {
    throw new UnsupportedOperationException("Replacing elements is not allowed in SortedVector");
  }

  public synchronized void setElementAt(Object obj, int index)
  {
    this.set(index, obj);
  }
}
