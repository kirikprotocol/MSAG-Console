/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 17:32:26
 */
package ru.novosoft.smsc.util.auth;

import org.apache.catalina.realm.RealmBase;

import java.security.Principal;

import ru.novosoft.smsc.admin.Constants;

public class SmscRealm extends RealmBase
{
	protected String getName()
	{
		return Constants.TomcatRealmName;
	}

	protected String getPassword(String s)
	{
		return null;
	}

	protected Principal getPrincipal(String s)
	{
		return null;
	}

	public Principal authenticate(String login, String password)
	{
		System.out.println("SmscRealm.authenticate(\"" + login + "\", \"" + password+"\")");
		return AuthenticatorProxy.getInstance().authenticate(getName(), login, password);
	}

	public boolean hasRole(Principal principal, String role)
	{
		System.out.println("SmscRealm.hasRole(\"" + principal.getName() + "\", \"" + role + "\")");
		return AuthenticatorProxy.getInstance().hasRole(getName(), principal, role);
	}
}
