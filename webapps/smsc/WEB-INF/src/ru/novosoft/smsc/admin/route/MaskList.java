/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 15:13:49
 */
package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.SortedList;

import java.io.PrintWriter;
import java.util.*;

import org.apache.log4j.Category;


public class MaskList
{
	private Map masks = new HashMap();
	private Category logger = Category.getInstance(this.getClass());

	public MaskList()
	{
	}

	public MaskList(String masks)
	{
		for (StringTokenizer tokenizer = new StringTokenizer(masks); tokenizer.hasMoreTokens();)
		{
			final String mask = tokenizer.nextToken();
			try
			{
				add(new Mask(mask));
			}
			catch (AdminException e)
			{
				logger.error("Couldn't load mask \"" + mask + "\", skipped", e);
			}
		}
	}

	public MaskList(String[] masks)
	{
		for (int i = 0; i < masks.length; i++)
		{
			String mask = masks[i].trim();
			try
			{
				add(new Mask(mask));
			}
			catch (AdminException e)
			{
				logger.error("Couldn't load mask \"" + mask + "\", skipped", e);
			}
		}
	}

	public MaskList(Collection masks)
	{
		for (Iterator i = masks.iterator(); i.hasNext();)
		{
			String mask = ((String) i.next()).trim();
			try
			{
				add(new Mask(mask));
			}
			catch (AdminException e)
			{
				logger.error("Couldn't load mask \"" + mask + "\", skipped", e);
			}
		}
	}

	public MaskList(Mask mask)
	{
		add(mask);
	}

	public Mask add(Mask newMask)
	{
		if (newMask == null)
			throw new NullPointerException("Mask is null");

		return (Mask) masks.put(newMask.getMask(), newMask);
	}

	public Mask get(String key)
	{
		return (Mask) masks.get(key);
	}

	public int size()
	{
		return masks.size();
	}

	public Iterator iterator()
	{
		return masks.values().iterator();
	}

	public String getText()
	{
		String result = "";
		for (Iterator i = masks.keySet().iterator(); i.hasNext();)
		{
			result += (String) i.next() + "<br>";
		}
		return result;
	}

	public SortedList getNames()
	{
		return new SortedList(masks.keySet());
	}

	public PrintWriter store(PrintWriter out)
	{
		for (Iterator i = iterator(); i.hasNext();)
		{
			((Mask) i.next()).store(out);
		}
		return out;
	}

	public boolean isEmpty()
	{
		return masks.size() == 0;
	}
}
