package ru.sibinco.scag.jaas;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;

public class XmlAuthenticator implements Authenticator, RoleMapper{

  private static final Logger log = Logger.getLogger(XmlAuthenticator.class);

  private Map<String, String> user2password;
  private Map<String, Set<String>> user2roles;

  public XmlAuthenticator(final File usersFile) throws ParserConfigurationException, SAXException, IOException {
    try {
      if (log.isInfoEnabled()) log.info("Try to initialize file '" + usersFile.getAbsolutePath() + "' ...");
      Document document = parse(new FileReader(usersFile));
      NodeList usersNodeList = document.getElementsByTagName("user");
      user2password = new HashMap<String, String>();
      user2roles = new HashMap<String, Set<String>>();
      for (int i = 0; i < usersNodeList.getLength(); i++) {
        Element userElem = (Element) usersNodeList.item(i);
        String name = userElem.getAttribute("login");
        String password = userElem.getAttribute("password");
        NodeList rolesNodeList = userElem.getElementsByTagName("role");
        Set<String> roles = new HashSet<String>();
        for (int j = 0; j < rolesNodeList.getLength(); j++) {
          Element roleElem = (Element) rolesNodeList.item(j);
          roles.add(roleElem.getAttribute("name"));
        }
        user2password.put(name, password);
        user2roles.put(name, roles);
      }
      if (log.isInfoEnabled()) log.info("Initialized success with " + user2password.values().size() + " users.");
    } catch (IOException e) {
      log.error("Couldn't load users from file "+usersFile+"'.", e);
      throw e;
    } catch (ParserConfigurationException e) {
      log.error("Couldn't load users from file "+usersFile+"'.", e);
      throw e;
    } catch (SAXException e) {
      log.error("Couldn't load users from file "+usersFile+"'.", e);
      throw e;
    }
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

    if (log.isDebugEnabled()) log.debug("Authenticate '" + login + "' with roles: " + user2roles.get(login));
    return true;
  }

  private Document parse(final Reader input) throws FactoryConfigurationError, ParserConfigurationException, SAXException, IOException, NullPointerException{
    if (null == input) throw new NullPointerException("input stream is null");
    final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
    final DocumentBuilder builder = factory.newDocumentBuilder();
    builder.setEntityResolver(new DtdsEntityResolver());
    final InputSource source = new InputSource(input);
    return builder.parse(source);
  }

  @Override
  public Set<String> getRoles(String userName) {
    return null;
  }

  private class DtdsEntityResolver implements EntityResolver{

    private String getDtdName(final String systemId){
      int index = systemId.lastIndexOf('/');
      if (0 > index)
        index = systemId.lastIndexOf('\\');
      if (0 <= index)
        return systemId.substring(index + 1);
      else
        return systemId;
    }

    public InputSource resolveEntity(final String publicId, final String systemId) throws SAXException, IOException{
      if (systemId.endsWith(".dtd")) {
        String filename = "dtds/" + getDtdName(systemId);
        InputStream in = Thread.currentThread().getContextClassLoader().getResourceAsStream(filename);
        if (in != null) return new InputSource(in);
      }
      return null;
    }
  }

}
