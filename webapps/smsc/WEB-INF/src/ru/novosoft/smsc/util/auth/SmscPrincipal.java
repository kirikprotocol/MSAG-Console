/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 22:38:32
 */
package ru.novosoft.smsc.util.auth;

import java.security.Principal;
import java.util.Set;

public final class SmscPrincipal implements Principal
{
	private String name = null;
	private String password = null;
	private Set roles = null;

	protected SmscPrincipal(String name, String password, Set roles)
	{
		this.name = name;
		this.password = password;
		this.roles = roles;
	}

	/**
	 * Returns the name of this principal.
	 *
	 * @return the name of this principal.
	 */
	public String getName()
	{
		return name;
	}

	protected void setName(String name)
	{
		this.name = name;
	}

	protected String getPassword()
	{
		return password;
	}

	protected Set getRoles()
	{
		return roles;
	}
}
