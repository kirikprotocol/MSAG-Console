/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 20:21:04
 */
package ru.novosoft.smsc.util.auth;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.util.xml.Utils;

import java.io.File;
import java.io.FileInputStream;
import java.security.Principal;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

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
		Document document = Utils.parse(new FileInputStream(config));
		NodeList usersNodeList = document.getElementsByTagName("user");
		users = new HashMap();
		for (int i = 0; i < usersNodeList.getLength(); i++)
		{
			Element userElem = (Element) usersNodeList.item(i);
			String name = userElem.getAttribute("name");
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
		AuthenticatorProxy.getInstance().registerAuthenticator("SMSC.SmscRealm", this);
	}

	public synchronized Principal authenticate(String login, String password)
	{
		if (users == null)
		{
			System.out.println("XmlAuthenticator.authenticate(\"" + login + "\", \"" + password + "\") FAILED - AUTHENTICATOR NOT INITIALIZED");
			return null;
		}

		SmscPrincipal principal = (SmscPrincipal) users.get(login);
		return principal != null && principal.getPassword().equals(password)
				  ? principal
				  : null;
	}

	public synchronized boolean hasRole(Principal principal, String role)
	{
		if (users == null)
			return false;

		SmscPrincipal ud = (SmscPrincipal) users.get(principal.getName());
		return ud != null && ud.getRoles().contains(role);
	}

}
