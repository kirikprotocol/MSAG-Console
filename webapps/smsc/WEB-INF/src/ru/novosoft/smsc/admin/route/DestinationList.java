/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 14:14:40
 */
package ru.novosoft.smsc.admin.route;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;

import java.io.PrintWriter;
import java.util.*;


public class DestinationList
{
	Map destinations = new HashMap();

	public DestinationList()
	{
	}

	public DestinationList(Element destinationListElement, SubjectList subjects, SMEList smes)
			throws AdminException
	{
		NodeList list = destinationListElement.getElementsByTagName("destination");
		for (int i = 0; i < list.getLength(); i++)
		{
			add(new Destination((Element) list.item(i), subjects, smes));
		}
	}

	public void add(Destination d)
	{
		if (d == null)
			throw new NullPointerException("Destination is null");
		if (destinations.containsKey(d.getName()))
			throw new IllegalArgumentException("Source already contained");

		destinations.put(d.getName(), d);
	}

	public int size()
	{
		return destinations.size();
	}

	public Destination get(String destinationName)
	{
		return (Destination) destinations.get(destinationName);
	}

	public Iterator iterator()
	{
		return destinations.values().iterator();
	}

	public boolean isEmpty()
	{
		return destinations.isEmpty();
	}

	public Set getNames()
	{
		return destinations.keySet();
	}

	public Destination remove(String name)
	{
		return (Destination) destinations.remove(name);
	}

	public boolean removeAll(DestinationList dsts)
	{
		return destinations.keySet().removeAll(dsts.getNames());
	}

	public void addAll(DestinationList dsts)
	{
		destinations.putAll(dsts.destinations);
	}

	public boolean retainAll(DestinationList dsts)
	{
		return destinations.keySet().retainAll(dsts.getNames());
	}

	public Set getSubjectNames()
	{
		Set result = new HashSet();
		for (Iterator i = destinations.values().iterator(); i.hasNext();)
		{
			Destination d = (Destination) i.next();
			if (d.isSubject())
				result.add(d.getName());
		}

		return result;
	}

	public Set getMaskNames()
	{
		Set result = new HashSet();
		for (Iterator i = destinations.values().iterator(); i.hasNext();)
		{
			Destination d = (Destination) i.next();
			if (!d.isSubject())
				result.addAll(d.getMasks().getNames());
		}

		return result;
	}

	public PrintWriter store(PrintWriter out)
	{
		for (Iterator i = iterator(); i.hasNext();)
		{
			((Destination) i.next()).store(out);
		}
		return out;
	}

	public boolean isSubjectUsed(String subjectId)
	{
		for (Iterator i = destinations.values().iterator(); i.hasNext();)
		{
			Destination destination = (Destination) i.next();
			if (destination.isSubject() && destination.getName().equals(subjectId))
				return true;
		}
		return false;
	}
}