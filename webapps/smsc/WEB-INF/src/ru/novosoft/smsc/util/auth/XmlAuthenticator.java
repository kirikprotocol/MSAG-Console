/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 20:21:04
 */
package ru.novosoft.smsc.util.auth;

import org.w3c.dom.*;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.util.xml.Utils;

import java.io.File;
import java.io.FileInputStream;
import java.security.Principal;
import java.util.*;

public class XmlAuthenticator implements Authenticator
{
	private static XmlAuthenticator instance = null;

	public static synchronized XmlAuthenticator getInstance()
	{
		if (instance == null)
			instance = new XmlAuthenticator();
		return instance;
	}


	private Map users = null;

	private XmlAuthenticator()
	{
	}

	public synchronized static void init(File config)
			  throws Exception
	{
		if (instance == null)
			instance = new XmlAuthenticator();
		instance.initialize(config);
	}

	private synchronized void initialize(File config)
			  throws Exception
	{
		try
		{
			System.err.println("XmlAuthenticator.initialize. File \"" + config.getAbsolutePath() + '"');
			Document document = Utils.parse(new FileInputStream(config));
			NodeList usersNodeList = document.getElementsByTagName("user");
			users = new HashMap();
			for (int i = 0; i < usersNodeList.getLength(); i++)
			{
				Element userElem = (Element) usersNodeList.item(i);
				String name = userElem.getAttribute("login");
				String password = userElem.getAttribute("password");
				NodeList rolesNodeList = userElem.getElementsByTagName("role");
				Set roles = new HashSet();
				for (int j = 0; j < rolesNodeList.getLength(); j++)
				{
					Element roleElem = (Element) rolesNodeList.item(j);
					roles.add(roleElem.getAttribute("name"));
				}
				users.put(name, new SmscPrincipal(name, password, roles));
			}
			AuthenticatorProxy.getInstance().registerAuthenticator(Constants.TomcatRealmName, this);
			System.err.println("XmlAuthenticator.initialize success with " + users.values().size() + " users.");
		}
		catch (Exception e)
		{
			System.err.println("XmlAuthenticator.initialize FAILED. Nested : " + e.getMessage());
			e.printStackTrace();
			throw e;
		}
	}

	public synchronized Principal authenticate(String login, String password)
	{
		if (users == null)
		{
			System.out.println("XmlAuthenticator.authenticate(\"" + login + "\", \"" + password + "\") FAILED - AUTHENTICATOR NOT INITIALIZED");
			return null;
		}

		SmscPrincipal principal = (SmscPrincipal) users.get(login);
		if (principal != null && principal.getPassword().equals(password))
		{
			System.out.println("XmlAuthenticator.authenticate(\"" + login + "\", \"" + password + "\") Success.");
			return principal;
		}
		else
		{
			if (principal == null)
				System.out.println("XmlAuthenticator.authenticate(\"" + login + "\", \"" + password + "\") FAILED - User not found.");
			else
				System.out.println("XmlAuthenticator.authenticate(\"" + login + "\", \"" + password + "\") FAILED - Incorrect password.");

			return null;
		}
	}

	public synchronized boolean hasRole(Principal principal, String role)
	{
		if (users == null)
			return false;

		SmscPrincipal ud = (SmscPrincipal) users.get(principal.getName());
		return ud != null && ud.getRoles().contains(role);
	}
}
