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

	public synchronized void registerAuthenticator(String realmName, Authenticator authenticator)
	{
		System.out.println("AuthenticatorProxy.registerAuthenticator(\"" + realmName + "\", \"" + authenticator + "\")");
		authenticators.put(realmName, authenticator);
	}

	public synchronized Principal authenticate(String realmName, String login, String password)
	{
		Authenticator auth = (Authenticator) authenticators.get(realmName);
		System.out.println("AuthenticatorProxy.authenticate(\"" + realmName + "\", \"" + login + "\", \"" + password+"\") auth="+auth);
		if (auth == null)
			return null;
		else
			return auth.authenticate(login, password);
	}

	public synchronized boolean hasRole(String realmName, Principal principal, String role)
	{
		System.out.println("AuthenticatorProxy.hasRole(\"" + realmName + "\", \"" + principal.getName() + "\", \"" + role + "\")");
		Authenticator auth = (Authenticator) authenticators.get(realmName);
		if (auth == null)
			return false;
		else
			return auth.hasRole(principal, role);
	}
}
