/*
 * Author: igork
 * Date: 05.06.2002
 * Time: 16:01:33
 */
package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.admin.route.Mask;


public class Alias
{
	private Mask address = null;
	private Mask alias = null;
	private boolean hide = false;
	private int hash = 0;

	public Alias(Mask address, Mask alias, boolean hide)
	{
		this.address = address;
		this.alias = alias;
		this.hide = hide;
		hash = (address.getMask() + '|' + alias.getMask()).hashCode();
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
		return hash;
	}

	public boolean equals(Object obj)
	{
		if (obj instanceof Alias)
		{
			Alias a = (Alias) obj;
			return address.equals(a.address) && alias.equals(a.alias);
		}
		else
		{
			return super.equals(obj);
		}
	}
}
