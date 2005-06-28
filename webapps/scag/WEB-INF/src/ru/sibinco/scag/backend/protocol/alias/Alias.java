package ru.sibinco.scag.backend.protocol.alias;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 16:26:34
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.SibincoException;


public class Alias
{
	private Mask address = null;
	private Mask alias = null;
	private boolean hide = false;

	public Alias(Mask address, Mask alias, boolean hide)
			throws SibincoException
	{
		int aliasQuestionsCount = alias.getQuestionsCount();
        int addressQuestionsCount = address.getQuestionsCount();
        if (hide && (addressQuestionsCount > 0 || aliasQuestionsCount > 0))
            throw new SibincoException("alias incorrect - question marks aren't allowed for aliases with hide option");
        if (addressQuestionsCount != aliasQuestionsCount)
			throw new SibincoException("alias incorrect - question marks in alias and address do not match");
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
			return false;
		}
	}
}
