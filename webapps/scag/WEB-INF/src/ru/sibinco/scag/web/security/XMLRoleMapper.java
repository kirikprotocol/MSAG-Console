package ru.sibinco.scag.web.security;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.util.*;
import java.util.regex.Pattern;

public class XMLRoleMapper implements RoleMapper{

  private static final Logger log = Logger.getLogger(XMLRoleMapper.class);

  private Map<String, Set<String>> user2roles;
  private Map<String, Set<String>> role2URIPatterns;

  public XMLRoleMapper(Document userXmlDocument, Document webXmlDocument){
    user2roles = getRoles(userXmlDocument);
    role2URIPatterns = getURIPatterns(webXmlDocument);
    if (log.isDebugEnabled()) log.debug("XML role mapper has been initialized.");
  }

  public XMLRoleMapper(Document userXmlDocument, Map<String, Set<String>> role2URIPatterns){
    user2roles = getRoles(userXmlDocument);
    this.role2URIPatterns = role2URIPatterns;
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

  private Map<String, Set<String>> getURIPatterns(Document webXmlDocument){
    Map<String, Set<String>> role2uris = new HashMap<String, Set<String>>();
    NodeList scList = webXmlDocument.getElementsByTagName("security-constraint");
    if (log.isDebugEnabled()) log.debug("Found "+scList.getLength()+" 'security-constraint' tags.");
    for (int i = 0; i < scList.getLength(); i++) {
      Node nNode = scList.item(i);
      if (nNode.getNodeType() == Node.ELEMENT_NODE) {
        Element scElement = (Element) nNode;
        NodeList childNodes = scElement.getChildNodes();
        if (log.isDebugEnabled()) log.debug("Found "+childNodes.getLength()+" children nodes in 'security-constraint' tag.");

        String role = null;
        Set<String> uriPatterns = new HashSet<String>();

        for (int j = 0; j < scList.getLength(); j++) {
          Node childNode = childNodes.item(j);
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
                    String urlPattern = childElement1.getTextContent();
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
                    role = childElement1.getTextContent();
                    if (log.isDebugEnabled()) log.debug("Found role: "+role);
                    break;
                  }
                }
              }
            }
          }
        }
        if (role != null) {
          role2uris.put(role, uriPatterns);
          if (log.isDebugEnabled()) log.debug("Added role --> uri patterns: "+role+" --> "+ Arrays.toString(uriPatterns.toArray()));
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
    Set<String> roles = new HashSet<String>();
    for(String role: role2URIPatterns.keySet()){
      Set<String> uriPatterns = role2URIPatterns.get(role);
      for(String uriPattern: uriPatterns){
        String regex = wildcardToRegex(uriPattern);
        //if (log.isDebugEnabled()) log.debug("uriPattern: '"+uriPattern+"', regex: '"+regex+"'");
        if (Pattern.matches(wildcardToRegex(uriPattern), uri)) roles.add(role);
      }
    }
    if (log.isDebugEnabled()) log.debug("Roles allowed for uri '"+uri+"': "+Arrays.toString(roles.toArray()));
    return roles;
  }

  @Override
  public Map<String, Set<String>> getURIsForRoles() {
    return role2URIPatterns;
  }

  public static String wildcardToRegex(String wildcard){
    StringBuffer s = new StringBuffer(wildcard.length());
    s.append('^');
    for (int i = 0, is = wildcard.length(); i < is; i++) {
      char c = wildcard.charAt(i);
      switch(c) {
        case '*':
          s.append(".*");
          break;
        case '?':
          s.append(".");
          break;
        // escape special regexp-characters
        case '(': case ')': case '[': case ']': case '$':
        case '^': case '.': case '{': case '}': case '|':
        case '\\':
          s.append("\\");
          s.append(c);
          break;
        default:
          s.append(c);
          break;
      }
    }
    s.append('$');
    return(s.toString());
  }

}
