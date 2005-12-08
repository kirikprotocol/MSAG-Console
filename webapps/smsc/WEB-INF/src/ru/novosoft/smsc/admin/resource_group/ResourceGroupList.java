package ru.novosoft.smsc.admin.resource_group;

import ru.novosoft.smsc.admin.resource_group.ResourceGroup;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.SortedList;

import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.Iterator;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 06.09.2005
 * Time: 15:02:37
 * To change this template use File | Settings | File Templates.
 */
public class ResourceGroupList
{
	private Map resourceGroups = new HashMap();

	public void add(ResourceGroup d)
			throws AdminException
	{
		if (resourceGroups.containsKey(d.getName()))
			throw new AdminException("Resource group \"" + d.getName() + "\" already contained in list");
		resourceGroups.put(d.getName(), d);
	}

	public ResourceGroup remove(String name) throws AdminException
	{
		require(name);
		return (ResourceGroup) resourceGroups.remove(name);
	}

	private void require(String name) throws AdminException
	{
		if (!resourceGroups.containsKey(name))
			throw new AdminException("Unknown resource group \"" + name + "\"");
	}

	public List getResourceGroupsNames()
	{
		return new SortedList(resourceGroups.keySet());
	}

	public ResourceGroup get(String name) throws AdminException
	{
		require(name);
		return (ResourceGroup) resourceGroups.get(name);
	}

	public boolean contains(String name)
	{
		return resourceGroups.containsKey(name);
	}

	public Iterator iterator()
	{
		return resourceGroups.values().iterator();
	}

	public void clear()
	{
		resourceGroups.clear();
	}
}
