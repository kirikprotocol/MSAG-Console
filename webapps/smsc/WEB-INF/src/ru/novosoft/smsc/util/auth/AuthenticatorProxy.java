/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 22:25:58
 */
package ru.novosoft.smsc.util.auth;

import java.security.Principal;
import java.util.HashMap;
import java.util.Map;

public class AuthenticatorProxy
{
	private static AuthenticatorProxy instance = null;

	public static synchronized AuthenticatorProxy getInstance()
	{
		if (instance == null)
			instance = new AuthenticatorProxy();

		return instance;
	}


	private Map authenticators = new HashMap();

	public void registerAuthenticator(String realmName, Authenticator authenticator)
	{
		synchronized (authenticators)
		{
			authenticators.put(realmName, authenticator);
		}
	}

	public Principal authenticate(String realmName, String login, String password)
	{
		Authenticator auth;
		synchronized (authenticators)
		{
			auth = (Authenticator) authenticators.get(realmName);
		}
		if (auth == null)
			return null;
		else
			return auth.authenticate(login, password);
	}

	public boolean hasRole(String realmName, Principal principal, String role)
	{
		Authenticator auth;
		synchronized (authenticators)
		{
			auth = (Authenticator) authenticators.get(realmName);
		}
		if (auth == null)
			return false;
		else
			return auth.hasRole(principal, role);
	}
}
