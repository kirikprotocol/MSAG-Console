/*
 * Created by igork
 * Date: 22.11.2002
 * Time: 20:21:04
 */
package ru.sibinco.tomcat_auth;

import org.w3c.dom.*;
import org.xml.sax.*;

import javax.xml.parsers.*;
import java.io.*;
import java.security.Principal;
import java.util.*;


public class XmlAuthenticator implements Authenticator
{
  private class DtdsEntityResolver implements EntityResolver
  {
    private String getDtdName(final String systemId)
    {
      int index = systemId.lastIndexOf('/');
      if (index < 0)
        index = systemId.lastIndexOf('\\');
      if (index >= 0)
        return systemId.substring(index + 1);
      else
        return systemId;
    }

    public InputSource resolveEntity(final String publicId, final String systemId) throws SAXException, IOException
    {
      if (systemId.endsWith(".dtd")) {
        final String filename = "dtds/" + getDtdName(systemId);
        final InputStream in = Thread.currentThread().getContextClassLoader().getResourceAsStream(filename);
        if (in != null) {
          final InputSource inputSource = new InputSource(in);
          return inputSource;
        }
      }
      return null;
    }
  }


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

  public synchronized static void init(final File config)
      throws Exception
  {
    if (instance == null)
      instance = new XmlAuthenticator();
    instance.initialize(config);
  }

  private Document parse(final Reader input) throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException, NullPointerException
  {
    if (input == null)
      throw new NullPointerException("input stream is null");
    final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
    final DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(new DtdsEntityResolver());
    final InputSource source = new InputSource(input);
    return builder.parse(source);
  }

  private synchronized void initialize(final File config)
      throws Exception
  {
    try {
      System.out.println("initialize. File \"" + config.getAbsolutePath() + '"');
      final Document document = parse(new FileReader(config));
      final NodeList usersNodeList = document.getElementsByTagName("user");
      users = new HashMap();
      for (int i = 0; i < usersNodeList.getLength(); i++) {
        final Element userElem = (Element) usersNodeList.item(i);
        final String name = userElem.getAttribute("login");
        final String password = userElem.getAttribute("password");
        final NodeList rolesNodeList = userElem.getElementsByTagName("role");
        final Set roles = new HashSet();
        for (int j = 0; j < rolesNodeList.getLength(); j++) {
          final Element roleElem = (Element) rolesNodeList.item(j);
          roles.add(roleElem.getAttribute("name"));
        }
        users.put(name, new SibincoPrincipal(name, password, roles));
      }
      AuthenticatorProxy.getInstance().registerAuthenticator(SibincoRealm.REALM_NAME, this);
      System.out.println("initialize success with " + users.values().size() + " users.");
    } catch (Exception e) {
      System.out.println("initialize FAILED. Nested : " + e.getMessage());
      e.printStackTrace();
      throw e;
    }
  }

  public synchronized Principal authenticate(final String login, final String password)
  {
    if (users == null) {
      System.out.println("authenticate(\"" + login + "\", \"" + password + "\") FAILED - AUTHENTICATOR NOT INITIALIZED");
      return null;
    }

    final SibincoPrincipal principal = (SibincoPrincipal) users.get(login);
    if (principal != null && principal.getPassword().equals(password)) {
      return principal;
    } else {
      if (principal == null)
        System.out.println("authenticate(\"" + login + "\", \"" + password + "\") FAILED - User not found.");
      else
        System.out.println("authenticate(\"" + login + "\", \"" + password + "\") FAILED - Incorrect password.");

      return null;
    }
  }

  public synchronized boolean hasRole(final Principal principal, final String role)
  {
    if (users == null)
      return false;

    final SibincoPrincipal ud = (SibincoPrincipal) users.get(principal.getName());
    if (ud != null) {
      final StringBuffer rolesString = new StringBuffer();
      for (Iterator i = ud.getRoles().iterator(); i.hasNext();) {
        final String roleName = (String) i.next();
        rolesString.append(roleName);
        if (i.hasNext()) rolesString.append(", ");
      }
    }
    return ud != null && ud.getRoles().contains(role);
  }
}
