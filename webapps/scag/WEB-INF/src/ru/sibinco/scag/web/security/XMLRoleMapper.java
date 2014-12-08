package ru.sibinco.scag.web.security;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
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

  public XMLRoleMapper(Document userXmlDocument, Map<String, Set<String>> role2uris){
    user2roles = getRoles(userXmlDocument);
    this.role2uris = role2uris;
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
    if (log.isDebugEnabled()) log.debug("Found "+scList.getLength()+" 'security-constraint' tags.");
    for (int i = 0; i < scList.getLength(); i++) {
      Node nNode = scList.item(i);
      if (nNode.getNodeType() == Node.ELEMENT_NODE) {
        Element scElement = (Element) nNode;
        NodeList childNodes = scElement.getChildNodes();
        if (log.isDebugEnabled()) log.debug("Found "+childNodes.getLength()+" children nodes in 'security-constraint' tag.");
        for (int j = 0; j < scList.getLength(); j++) {
          Node childNode = childNodes.item(j);
          String role = null;
          Set<String> uriPatterns = new HashSet<String>();
          if (childNode.getNodeType() == Node.ELEMENT_NODE){
            Element childElement = (Element) childNode;
            String tagName = childElement.getTagName();
            if (tagName.equals("web-resource-collection")){
              if (log.isDebugEnabled()) log.debug("Found 'web-resource-collection' tag.");
              NodeList childNodes1 = childElement.getChildNodes();
              for (int k = 0; k < childNodes1.getLength(); k++) {
                Node childNode1 = childNodes1.item(k);
                if (childNode1.getNodeType() == Node.ELEMENT_NODE){
                  Element childElement1 = (Element) childNode1;
                  if (childElement1.getTagName().equals("url-pattern")){
                    String urlPattern = childElement1.getNodeValue();
                    if (log.isDebugEnabled()) log.debug("Found url-pattern: "+urlPattern);
                    uriPatterns.add(urlPattern);
                  }
                }
              }
            }
            if (tagName.equals("auth-constraint")){
              if (log.isDebugEnabled()) log.debug("Found 'auth-constraint' tag.");
              NodeList childNodes1 = childElement.getChildNodes();
              for (int k = 0; k < childNodes1.getLength(); k++) {
                Node childNode1 = childNodes1.item(k);
                if (childNode1.getNodeType() == Node.ELEMENT_NODE){
                  Element childElement1 = (Element) childNode1;
                  if (childElement1.getTagName().equals("role-name")){
                    role = childElement1.getNodeValue();
                    if (log.isDebugEnabled()) log.debug("Found role: "+role);
                    break;
                  }
                }
              }
            }
          }
          if (role != null) {
            role2uris.put(role, uriPatterns);
            if (log.isDebugEnabled()) log.debug("Added role --> uri patterns: "+role+" --> "+uriPatterns);
          }
        }
      }
    }
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

  @Override
  public Map<String, Set<String>> getURIsForRoles() {
    return role2uris;
  }

}
