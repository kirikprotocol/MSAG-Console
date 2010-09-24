package mobi.eyeline.informer.util.xml;

import mobi.eyeline.informer.util.XmlUtils;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerConfigurationException;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
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

  private Document document;

  private UrlPatterns urlPatterns;

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  private final Set<String> roles = new TreeSet<String>();

  public WebXml(InputStream is) throws TransformerConfigurationException, ParserConfigurationException, SAXException, IOException {
    load(is);
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public WebXml(File webXmlFile) throws TransformerConfigurationException, ParserConfigurationException, SAXException, IOException {
    InputStream is = null;
    try{
      is = new FileInputStream(webXmlFile);
      load(is);
    }finally {
      if(is != null) {
        try{
        is.close();
        }catch (IOException e){}
      }
    }
  }

  private void load(InputStream is) throws IOException, SAXException, ParserConfigurationException {
    document = XmlUtils.parse(is);
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
    return new HashSet<String>(roles);
  }

  public Collection<String> getRoles(String url) {
    try {
      lock.readLock().lock();
      return urlPatterns.getRoles(url);
    } finally {
      lock.readLock().unlock();
    }
  }


  private Element createRole(String roleName) {
    Element securityRoleElement = document.createElement("security-role");
    Element roleNameElement = XmlUtils.createChildElement(document, securityRoleElement, "role-name");
    XmlUtils.createTextChild(document, roleNameElement, roleName);
    return securityRoleElement;
  }



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

    private final Map<String, UrlPattern> exactMatches = new HashMap<String, UrlPattern>();

    private final Map<String, UrlPattern> pathPatterns = new HashMap<String, UrlPattern>();

    private final Map<String, UrlPattern> extensionsPatterns = new HashMap<String, UrlPattern>();

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
