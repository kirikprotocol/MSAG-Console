/*
 * Author: igork
 * Date: 05.06.2002
 * Time: 16:01:33
 */
package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;


public class Alias
{
	private Mask address = null;
	private Mask alias = null;
	private boolean hide = false;

	public Alias(Mask address, Mask alias, boolean hide)
			throws AdminException
	{
		if (address.getQuestionsCount() != alias.getQuestionsCount())
			throw new AdminException("alias incorrect - question marks in alias and address do not match");
		this.address = address;
		this.alias = alias;
		this.hide = hide;
	}

	public Mask getAddress()
	{
		return address;
	}

	public Mask getAlias()
	{
		return alias;
	}

	public boolean isHide()
	{
		return hide;
	}

	public int hashCode()
	{
		return alias.getMask().hashCode();
	}

	public boolean equals(Object obj)
	{
		if (obj instanceof Alias)
		{
			Alias a = (Alias) obj;
			return alias.equals(a.alias);
		}
		else
		{
			return super.equals(obj);
		}
	}
}
