package ru.novosoft.smsc.jsp.smsc.localeResources;

import ru.novosoft.smsc.util.SortedList;

import java.util.*;

/**
 * Created by igork
 * Date: 12.05.2003
 * Time: 19:09:43
 */
public class Section implements Comparable
{
	public static final char NAME_DELIMETER = '.';
	private String name = null;
	private Section parent;
	private Map sections = new HashMap();
	private Map params = new HashMap();

	public Section(String name, Section parent)
	{
		this.name = name;
		this.parent = parent;
	}

	public String getName()
	{
		return name;
	}

	public String getFullName()
	{
		if (parent != null)
			return parent.getFullName() + NAME_DELIMETER + name;
		else
			return name;
	}

	public List getSectionNames()
	{
		return new SortedList(sections.keySet());
	}

	public Section getSection(String sectionName)
	{
		return (Section) sections.get(sectionName);
	}

	public List getParamNames()
	{
		return new SortedList(params.keySet());
	}

	public String getParam(String paramName)
	{
		return (String) params.get(paramName);
	}

	public void setParam(String paramName, String paramValue)
	{
		params.put(paramName, paramValue);
	}

	public int hashCode()
	{
		return name.hashCode();
	}

	public boolean equals(Object obj)
	{
		if (obj instanceof Section)
		{
			Section s = (Section) obj;
			return name.equals(s.name);
		}
		else
			return false;
	}

	public int compareTo(Object o)
	{
		if (o instanceof Section)
		{
			Section s = (Section) o;
			return name.compareTo(s.name);
		}
		else
			return 0;
	}

	private Section getOrCreateSection(String sectionName)
	{
		Section result = (Section) sections.get(sectionName);
		if (result == null)
		{
			result = new Section(sectionName, this);
			sections.put(sectionName, result);
		}
		return result;
	}

	public Section findSection(String subSectionName)
	{
		final int pos = subSectionName.indexOf(NAME_DELIMETER);
		if (pos >= 0)
			return getOrCreateSection(subSectionName.substring(0, pos)).findSection(subSectionName.substring(pos + 1));
		else
			return getOrCreateSection(subSectionName);
	}

	public Section getParent()
	{
		return parent;
	}
}
