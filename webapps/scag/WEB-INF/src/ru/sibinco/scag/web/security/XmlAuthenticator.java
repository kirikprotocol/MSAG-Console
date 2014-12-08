package ru.sibinco.scag.web.security;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;

public class XmlAuthenticator implements Authenticator{

  private static final Logger log = Logger.getLogger(XmlAuthenticator.class);

  private Map<String, String> user2password;

  public XmlAuthenticator(Document document) throws ParserConfigurationException, SAXException, IOException {
    user2password = getPasswordFromDocument(document);
  }

  private Map<String, String> getPasswordFromDocument(Document document){
    Map<String, String> user2password = new HashMap<String, String>();
    NodeList usersNodeList = document.getElementsByTagName("user");
    for (int i = 0; i < usersNodeList.getLength(); i++) {
      Element userElem = (Element) usersNodeList.item(i);
      String name = userElem.getAttribute("login");
      String password = userElem.getAttribute("password");
      user2password.put(name, password);
    }
    if (log.isInfoEnabled()) log.info("Initialized success with " + user2password.values().size() + " users.");
    return user2password;
  }

  public boolean authenticate(final String login, final String password){
    String configuredPassword = user2password.get(login);
    if (configuredPassword == null) {
      log.warn("XmlAuthenticator.authenticate('" + login + "', '" + password + "') FAILED - User not found.");
      return false;
    }

    if (!password.equals(configuredPassword)) {
      log.warn("XmlAuthenticator.authenticate('" + login + "', '" + password + "') FAILED - Incorrect password.");
      return false;
    }

    if (log.isDebugEnabled()) log.debug("User with login '" + login + "' has been authenticated.");
    return true;
  }

}
