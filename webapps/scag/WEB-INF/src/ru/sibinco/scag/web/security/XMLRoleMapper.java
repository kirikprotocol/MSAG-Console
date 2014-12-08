package ru.sibinco.scag.web.security;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class XMLRoleMapper implements RoleMapper{

  private static final Logger log = Logger.getLogger(XMLRoleMapper.class);

  private Map<String, Set<String>> user2roles;
  private Map<String, Set<String>> role2uris;

  public XMLRoleMapper(Document userXmlDocument, Document webXmlDocument){
    user2roles = getRoles(userXmlDocument);
    role2uris = getURIs(webXmlDocument);
    if (log.isDebugEnabled()) log.debug("XML role mapper has been initialized.");
  }

  private Map<String, Set<String>> getRoles(Document userXmlDocument){
    Map<String, Set<String>> user2roles = new HashMap<String, Set<String>>();
    NodeList usersNodeList = userXmlDocument.getElementsByTagName("user");
    for (int i = 0; i < usersNodeList.getLength(); i++) {
      Element userElem = (Element) usersNodeList.item(i);
      String username = userElem.getAttribute("login");
      NodeList rolesNodeList = userElem.getElementsByTagName("role");
      Set<String> roles = new HashSet<String>();
      for (int j = 0; j < rolesNodeList.getLength(); j++) {
        Element roleElem = (Element) rolesNodeList.item(j);
        String role = roleElem.getAttribute("name");
        roles.add(role);
        if (log.isDebugEnabled()) log.debug("Added username --> role: '"+username+"' --> '"+role+"'.");
      }
      user2roles.put(username, roles);
    }
    return user2roles;
  }

  private Map<String, Set<String>> getURIs(Document webXmlDocument){
    Map<String, Set<String>> role2uris = new HashMap<String, Set<String>>();
    NodeList scList = webXmlDocument.getElementsByTagName("security-constraint");

    return role2uris;
  }

  @Override
  public Set<String> getUserRoles(String username) {
    return user2roles.get(username);
  }

  @Override
  public Set<String> getRolesAllowedForURI(String uri) {
    return role2uris.get(uri);
  }

}
