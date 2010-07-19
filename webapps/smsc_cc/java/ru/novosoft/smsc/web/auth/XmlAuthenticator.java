/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.novosoft.smsc.web.auth;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.util.XmlUtils;
import ru.novosoft.smsc.web.InitException;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class XmlAuthenticator implements Authenticator {

  private static final Logger logger = Logger.getLogger(XmlAuthenticator.class);

  private Map<String, SmscPrincipal> users = null;

  public XmlAuthenticator(File config) throws InitException {
    reload(config);
  }

  public XmlAuthenticator(InputStream is) throws InitException {
    reload(is);
  }

  public void reload(InputStream is) throws InitException {
    try {
      if (logger.isDebugEnabled()) {
        logger.debug("XmlAuthenticator initialize");
      }
      Document document = XmlUtils.parse(is);
      NodeList usersNodeList = document.getElementsByTagName("user");
      Map<String, SmscPrincipal> users = new HashMap<String, SmscPrincipal>();
      for (int i = 0; i < usersNodeList.getLength(); i++) {
        Element userElem = (Element) usersNodeList.item(i);
        String name = userElem.getAttribute("login");
        String password = userElem.getAttribute("password");
        NodeList rolesNodeList = userElem.getElementsByTagName("role");
        Set<String> roles = new HashSet<String>(rolesNodeList.getLength());
        for (int j = 0; j < rolesNodeList.getLength(); j++) {
          Element roleElem = (Element) rolesNodeList.item(j);
          roles.add(roleElem.getAttribute("name"));
        }
        users.put(name, new SmscPrincipal(name, password, roles));
      }
      this.users = users;
      if (logger.isDebugEnabled()) {
        logger.debug("XmlAuthenticator.initialize success with " + users.values().size() + " users.");
      }
    } catch (ParserConfigurationException e) {
      logger.error(e, e);
      throw new InitException(e);
    } catch (FileNotFoundException e) {
      logger.error(e, e);
      throw new InitException(e);
    } catch (SAXException e) {
      logger.error(e, e);
      throw new InitException(e);
    } catch (IOException e) {
      logger.error(e, e);
      throw new InitException(e);
    }
  }


  public void reload(File config) throws InitException {
    InputStream is = null;
    try {
      is = new FileInputStream(config);
      reload(is);
    } catch (IOException e) {
      throw new InitException(e);
    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException e) {
        }
      }
    }
  }

  public synchronized SmscPrincipal authenticate(String login, String password) {
    if (users == null) {
      logger.error("XmlAuthenticator.authenticate('" + login + "', '" + password + "') FAILED - AUTHENTICATOR NOT INITIALIZED");
      return null;
    }

    SmscPrincipal principal = users.get(login);
    if (principal != null && principal.getPassword().equals(password)) {
      return principal;
    } else {
      if (principal == null)
        logger.warn("XmlAuthenticator.authenticate('" + login + "', '" + password + "') FAILED - User not found.");
      else
        logger.warn("XmlAuthenticator.authenticate('" + login + "', '" + password + "') FAILED - Incorrect password.");
      return null;
    }
  }

  public synchronized boolean hasRole(SmscPrincipal principal, String role) {
    if (users == null)
      return false;

    SmscPrincipal ud = users.get(principal.getName());
    return ud != null && ud.getRoles().contains(role);
  }
}