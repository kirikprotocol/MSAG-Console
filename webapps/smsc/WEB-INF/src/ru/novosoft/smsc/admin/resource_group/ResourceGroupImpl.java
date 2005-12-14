package ru.novosoft.smsc.admin.resource_group;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 09.09.2005
 * Time: 15:40:03
 * To change this template use File | Settings | File Templates.
 */

public abstract class ResourceGroupImpl implements ResourceGroup
{
	protected String name;

	protected String[] nodes;

	protected Category logger = Category.getInstance(this.getClass().getName());

	public String getName()
	{
		return name;
	}
}
