package ru.novosoft.smsc.util;

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
		this(new Comparator()
		{
			public int compare(Object o1, Object o2)
			{
				if (o1.equals(o2))
					return 0;
				if (o1 instanceof Comparable)
					return ((Comparable) o1).compareTo(o2);
				if (o2 instanceof Comparable)
					return ((Comparable) o2).compareTo(o1);
				if (o1 instanceof String && o2 instanceof String)
				{
					String s1 = (String) o1;
					String s2 = (String) o2;
					return s1.compareTo(s2);
				}
				return 0;
			}
		});
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
