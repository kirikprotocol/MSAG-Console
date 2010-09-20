package mobi.eyeline.informer.util.xml;

import mobi.eyeline.informer.util.XmlUtils;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerConfigurationException;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.regex.Pattern;

/**
 * web.xml parser
 * <p/>
 * User: igork
 * Date: 08.12.2003
 * Time: 20:27:07
 */
public class WebXml {

  private final File file;
  private final Document document;
//  private final Transformer transformer;
//  private static final String ESME_ROLE_NAME_PREFIX = "role_esme_";

  private UrlPatterns urlPatterns;

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private Set<String> roles = new TreeSet<String>();

  public WebXml(File webXmlFile) throws TransformerConfigurationException, ParserConfigurationException, SAXException, IOException {
    file = webXmlFile;
    document = XmlUtils.parse(file.getAbsolutePath());
//    transformer = TransformerFactory.newInstance().newTransformer();
    initRoles();
    initSecurityConstrains();
  }

  private void initRoles() {
    NodeList srs = document.getElementsByTagName("security-role");
    for (int i = 0; i < srs.getLength(); i++) {
      Element sc = (Element) srs.item(i);
      NodeList rs = sc.getElementsByTagName("role-name");
      for (int j = 0; j < rs.getLength(); j++) {
        Element r = (Element) rs.item(j);
        roles.add(r.getTextContent());
      }
    }
  }

  private void initSecurityConstrains() {
    NodeList scs = document.getElementsByTagName("security-constraint");
    Map<String, Set<String>> rolesByUrl = new HashMap<String, Set<String>>();

    for (int i = 0; i < scs.getLength(); i++) {
      Element sc = (Element) scs.item(i);
      NodeList acs = sc.getElementsByTagName("auth-constraint");
      Set<String> roles = new HashSet<String>();
      for (int j = 0; j < acs.getLength(); j++) {
        Element ac = (Element) acs.item(j);
        NodeList rs = ac.getElementsByTagName("role-name");
        for (int k = 0; k < rs.getLength(); k++) {
          Element role = (Element) rs.item(k);
          if(role.getTextContent().equals("*")) {
            roles.addAll(this.roles);
            break;
          }
          roles.add(role.getTextContent());
        }
      }
      NodeList resColls = sc.getElementsByTagName("web-resource-collection");
      for (int j = 0; j < resColls.getLength(); j++) {
        Element resColl = (Element) resColls.item(j);
        NodeList urls = resColl.getElementsByTagName("url-pattern");
        for (int k = 0; k < urls.getLength(); k++) {
          Element url = (Element) urls.item(k);
          rolesByUrl.put(url.getTextContent(), roles);
        }
      }
    }
    urlPatterns = new UrlPatterns(rolesByUrl);
  }

  public Set<String> getRoles() {
    return roles;
  }

  public Collection<String> getRoles(String url) {
    try {
      lock.readLock().lock();
      return urlPatterns.getRoles(url);
    } finally {
      lock.readLock().unlock();
    }
  }

//  public void save() throws IOException, TransformerException {
//    transformer.setOutputProperty(OutputKeys.DOCTYPE_PUBLIC, document.getDoctype().getPublicId());
//    transformer.setOutputProperty(OutputKeys.DOCTYPE_SYSTEM, document.getDoctype().getSystemId());
//    transformer.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
//    transformer.setOutputProperty(OutputKeys.INDENT, "yes");
//    transformer.setOutputProperty(OutputKeys.METHOD, "xml");
//    transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "no");
//
//    transformer.transform(new DOMSource(document), new StreamResult(file));
//  }

//  public synchronized String addSecurityConstraint(String serviceId) {
//    final String roleName = createRoleName(serviceId);
//
//    Element securityConstraintElement = createSecurityConstraint(serviceId, roleName);
//    Element securityRoleElement = createRole(roleName);
//    XmlUtils.appendFirstByTagName(document.getDocumentElement(), securityConstraintElement);
//    XmlUtils.appendFirstByTagName(document.getDocumentElement(), securityRoleElement);
//
//    return roleName;
//  }

//  public synchronized String removeSecurityConstraint(String serviceId) {
//    final String roleName = createRoleName(serviceId);
//    Element securityConstraintElement = findSecurityConstraint(serviceId);
//    Element securityRoleElement = findSecurityRole(roleName);
//    if (securityConstraintElement != null) {
//      securityConstraintElement.getParentNode().removeChild(securityConstraintElement);
//      String urlP = createUrlPattern(serviceId);
//      try {
//        lock.writeLock().lock();
//        urlPatterns.removePattern(urlP);
//      } finally {
//        lock.writeLock().unlock();
//      }
//    }
//    if (securityRoleElement != null)
//      securityRoleElement.getParentNode().removeChild(securityRoleElement);
//    return roleName;
//  }
//
//  public synchronized Collection<String> getEsmeRoleNames() {
//    Collection<String> result = new TreeSet<String>();
//    NodeList roleNameElements = document.getElementsByTagName("role-name");
//    for (int i = 0; i < roleNameElements.getLength(); i++) {
//      final Node roleNameNode = roleNameElements.item(i);
//      final Node parentNode = roleNameNode.getParentNode();
//      if (parentNode.getNodeType() == Node.ELEMENT_NODE) {
//        Element parentElement = (Element) parentNode;
//        if (parentElement.getTagName().equals("security-role")) {
//          final String roleName = XmlUtils.getNodeText(roleNameNode).trim();
//          if (roleName.startsWith(ESME_ROLE_NAME_PREFIX)) {
//            result.add(roleName);
//          }
//        }
//      }
//    }
//    return result;
//  }

  ///////////////////// helpers //////////////////

//  private Element findSecurityConstraint(String serviceId) {
//    final String urlPattern = createUrlPattern(serviceId);
//    NodeList urlPatternElements = document.getElementsByTagName("url-pattern");
//    for (int i = 0; i < urlPatternElements.getLength(); i++) {
//      final Node urlPatternNode = urlPatternElements.item(i);
//      String findedUrlPattern = XmlUtils.getNodeText(urlPatternNode).trim();
//      if (findedUrlPattern.equals(urlPattern))
//        return (Element) urlPatternNode.getParentNode().getParentNode();
//    }
//    return null;
//  }
//
//  private Element findSecurityRole(String roleName) {
//    NodeList roleNameElements = document.getElementsByTagName("role-name");
//    for (int i = 0; i < roleNameElements.getLength(); i++) {
//      final Node roleNameNode = roleNameElements.item(i);
//      final Node parentNode = roleNameNode.getParentNode();
//      if (parentNode.getNodeType() == Node.ELEMENT_NODE) {
//        Element parentElement = (Element) parentNode;
//        if (parentElement.getTagName().equals("security-role") && XmlUtils.getNodeText(roleNameNode).trim().equals(roleName)) {
//          return parentElement;
//        }
//      }
//    }
//    return null;
//  }


  private Element createRole(String roleName) {
    Element securityRoleElement = document.createElement("security-role");
    Element roleNameElement = XmlUtils.createChildElement(document, securityRoleElement, "role-name");
    XmlUtils.createTextChild(document, roleNameElement, roleName);
    return securityRoleElement;
  }

//  private Element createSecurityConstraint(String serviceId, final String roleName) {
//    Element securityConstraintElement = document.createElement("security-constraint");
//
//    Element displayName = XmlUtils.createChildElement(document, securityConstraintElement, "display-name");
//    XmlUtils.createTextChild(document, displayName, "Security constraint for ESME \"" + serviceId + '"');
//
//    Element webResourceCollectionElement = XmlUtils.createChildElement(document, securityConstraintElement, "web-resource-collection");
//    Element webResourceNameElement = XmlUtils.createChildElement(document, webResourceCollectionElement, "web-resource-name");
//    Element urlPatternElement = XmlUtils.createChildElement(document, webResourceCollectionElement, "url-pattern");
//    XmlUtils.createTextChild(document, webResourceNameElement, "Web resource for ESME \"" + serviceId + '"');
//    String urlPattern = createUrlPattern(serviceId);
//    XmlUtils.createTextChild(document, urlPatternElement, urlPattern);
//
//    Element authConstraintElement = XmlUtils.createChildElement(document, securityConstraintElement, "auth-constraint");
//    Element roleNameElement = XmlUtils.createChildElement(document, authConstraintElement, "role-name");
//    XmlUtils.createTextChild(document, roleNameElement, roleName);
//
//    Element userDataConstraintElement = XmlUtils.createChildElement(document, securityConstraintElement, "user-data-constraint");
//    Element transportGuaranteeElement = XmlUtils.createChildElement(document, userDataConstraintElement, "transport-guarantee");
//    XmlUtils.createTextChild(document, transportGuaranteeElement, "NONE");
//    try {
//      lock.writeLock().lock();
//      urlPatterns.addPattern(urlPattern, new ArrayList<String>(1) {{
//        add(roleName);
//      }});
//    } finally {
//      lock.writeLock().unlock();
//    }
//    return securityConstraintElement;
//  }

//  public static String createRoleName(String serviceId) {
//    return ESME_ROLE_NAME_PREFIX + serviceId;
//  }

//  public static String getServiceIdFromRole(String roleName) {
//    return roleName != null && roleName.length() > ESME_ROLE_NAME_PREFIX.length() ? roleName.substring(ESME_ROLE_NAME_PREFIX.length()) : null;
//  }

//  public static String createUrlPattern(String serviceId) {
//    return "/smsc/esme_" + serviceId + "/*";
//  }

  private static class UrlPattern {

    private String strPattern;

    private Pattern pattern = null;

    private Set<String> roles = new HashSet<String>();


    private boolean matches(String url) {
      if (pattern == null) {
        return strPattern.equals(url);
      }
      return pattern.matcher(url).matches();

    }
  }

  private static class UrlPatterns {

    private Map<String, UrlPattern> exactMatches = new HashMap<String, UrlPattern>();

    private Map<String, UrlPattern> pathPatterns = new HashMap<String, UrlPattern>();

    private Map<String, UrlPattern> extensionsPatterns = new HashMap<String, UrlPattern>();

    private UrlPatterns(Map<String, Set<String>> roles) {
      for (Map.Entry<String, Set<String>> e : roles.entrySet()) {
        addPattern(e.getKey(), e.getValue());
      }
    }

    private void addPattern(String strPattern, Collection<String> roles) {
      int index = strPattern.indexOf("*");
      UrlPattern p = new UrlPattern();
      p.roles = new HashSet<String>(roles);
      p.strPattern = strPattern;
      if (index == -1) {
        exactMatches.put(strPattern, p);
      } else if (index == 0) {
        p.pattern = Pattern.compile(strPattern.replace(".", "\\.").replace("*", ".*"));
        extensionsPatterns.put(strPattern, p);
      } else {
        p.pattern = Pattern.compile(strPattern.replace(".", "\\.").replace("*", ".*"));
        pathPatterns.put(p.strPattern, p);
      }
    }

//    private void removePattern(String strPattern) {
//      if (exactMatches.remove(strPattern) == null && pathPatterns.remove(strPattern) == null) {
//        extensionsPatterns.remove(strPattern);
//      }
//    }

    private Collection<String> getRoles(String url) {
      List<String> roles = null;
      for (UrlPattern pattern : exactMatches.values()) {
        if (pattern.matches(url)) {
          roles = new ArrayList<String>(pattern.roles);
          break;
        }
      }
      if(roles == null) {
        UrlPattern pattern = null;
        for (UrlPattern p : pathPatterns.values()) {
          if (p.matches(url)) {
            if (pattern == null || p.strPattern.length() > pattern.strPattern.length()) {
              pattern = p;
            }
          }
        }
        if (pattern != null) {
          roles = new ArrayList<String>(pattern.roles);
        }else {
          for (UrlPattern p : extensionsPatterns.values()) {
            if (p.matches(url)) {
              roles = new ArrayList<String>(p.roles);
              break;
            }
          }
        }
      }
      return roles == null ? null : roles;
    }

  }
}
