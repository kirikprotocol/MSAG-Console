package ru.novosoft.smsc.util.xml;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.*;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import java.io.File;
import java.io.IOException;
import java.util.*;

/**
 * web.xml parser
 *
 * User: igork
 * Date: 08.12.2003
 * Time: 20:27:07
 */
public class WebXml {

  private final File file;
  private final Document document;
  private final Transformer transformer;
  private static final String ROLE_NAME_PREFIX = "role_esme_";

  public WebXml(File webXmlFile) throws TransformerConfigurationException, ParserConfigurationException, SAXException, IOException {
    file = webXmlFile;
    document = Utils.parse(file.getAbsolutePath());
    transformer = TransformerFactory.newInstance().newTransformer();
  }

  public void save() throws IOException, TransformerException {
    transformer.setOutputProperty(OutputKeys.DOCTYPE_PUBLIC, document.getDoctype().getPublicId());
    transformer.setOutputProperty(OutputKeys.DOCTYPE_SYSTEM, document.getDoctype().getSystemId());
    transformer.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
    transformer.setOutputProperty(OutputKeys.INDENT, "yes");
    transformer.setOutputProperty(OutputKeys.METHOD, "xml");
    transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "no");

    transformer.transform(new DOMSource(document), new StreamResult(file));
  }

  public String addSecurityConstraint(String serviceId) {
    final String roleName = createRoleName(serviceId);

    Element securityConstraintElement = createSecurityConstraint(serviceId, roleName);
    Element securityRoleElement = createRole(roleName);
    Utils.appendFirstByTagName(document.getDocumentElement(), securityConstraintElement);
    Utils.appendFirstByTagName(document.getDocumentElement(), securityRoleElement);

    return roleName;
  }

  public String removeSecurityConstraint(String serviceId) {
    final String roleName = createRoleName(serviceId);
    Element securityConstraintElement = findSecurityConstraint(serviceId);
    Element securityRoleElement = findSecurityRole(roleName);
    if (securityConstraintElement != null)
      securityConstraintElement.getParentNode().removeChild(securityConstraintElement);
    if (securityRoleElement != null)
      securityRoleElement.getParentNode().removeChild(securityRoleElement);
    return roleName;
  }

  public Collection getRoleNames() {
    Collection result = new TreeSet();
    NodeList roleNameElements = document.getElementsByTagName("role-name");
    for (int i = 0; i < roleNameElements.getLength(); i++) {
      final Node roleNameNode = roleNameElements.item(i);
      final Node parentNode = roleNameNode.getParentNode();
      if (parentNode.getNodeType() == Node.ELEMENT_NODE) {
        Element parentElement = (Element) parentNode;
        if (parentElement.getTagName().equals("security-role")) {
          final String roleName = Utils.getNodeText(roleNameNode).trim();
          if (roleName.startsWith(ROLE_NAME_PREFIX)) {
            result.add(roleName);
          }
        }
      }
    }
    return result;
  }

  ///////////////////// helpers //////////////////

  private Element findSecurityConstraint(String serviceId) {
    final String urlPattern = createUrlPattern(serviceId);
    NodeList urlPatternElements = document.getElementsByTagName("url-pattern");
    for (int i = 0; i < urlPatternElements.getLength(); i++) {
      final Node urlPatternNode = urlPatternElements.item(i);
      String findedUrlPattern = Utils.getNodeText(urlPatternNode).trim();
      if (findedUrlPattern.equals(urlPattern))
        return (Element) urlPatternNode.getParentNode().getParentNode();
    }
    return null;
  }

  private Element findSecurityRole(String roleName) {
    NodeList roleNameElements = document.getElementsByTagName("role-name");
    for (int i = 0; i < roleNameElements.getLength(); i++) {
      final Node roleNameNode = roleNameElements.item(i);
      final Node parentNode = roleNameNode.getParentNode();
      if (parentNode.getNodeType() == Node.ELEMENT_NODE) {
        Element parentElement = (Element) parentNode;
        if (parentElement.getTagName().equals("security-role") && Utils.getNodeText(roleNameNode).trim().equals(roleName)) {
          return parentElement;
        }
      }
    }
    return null;
  }


  private Element createRole(String roleName) {
    Element securityRoleElement = document.createElement("security-role");
    Element roleNameElement = Utils.createChildElement(document, securityRoleElement, "role-name");
    Utils.createTextChild(document, roleNameElement, roleName);
    return securityRoleElement;
  }

  private Element createSecurityConstraint(String serviceId, final String roleName) {
    Element securityConstraintElement = document.createElement("security-constraint");

    Element displayName = Utils.createChildElement(document, securityConstraintElement, "display-name");
    Utils.createTextChild(document, displayName, "Security constraint for ESME \"" + serviceId + '"');

    Element webResourceCollectionElement = Utils.createChildElement(document, securityConstraintElement, "web-resource-collection");
    Element webResourceNameElement = Utils.createChildElement(document, webResourceCollectionElement, "web-resource-name");
    Element urlPatternElement = Utils.createChildElement(document, webResourceCollectionElement, "url-pattern");
    Utils.createTextChild(document, webResourceNameElement, "Web resource for ESME \"" + serviceId + '"');
    Utils.createTextChild(document, urlPatternElement, createUrlPattern(serviceId));

    Element authConstraintElement = Utils.createChildElement(document, securityConstraintElement, "auth-constraint");
    Element roleNameElement = Utils.createChildElement(document, authConstraintElement, "role-name");
    Utils.createTextChild(document, roleNameElement, roleName);

    Element userDataConstraintElement = Utils.createChildElement(document, securityConstraintElement, "user-data-constraint");
    Element transportGuaranteeElement = Utils.createChildElement(document, userDataConstraintElement, "transport-guarantee");
    Utils.createTextChild(document, transportGuaranteeElement, "NONE");

    return securityConstraintElement;
  }

  public static String createRoleName(String serviceId) {
    return ROLE_NAME_PREFIX + serviceId;
  }

  public static String getServiceIdFromRole(String roleName) {
    return roleName != null && roleName.length() > ROLE_NAME_PREFIX.length() ? roleName.substring(ROLE_NAME_PREFIX.length()) : null;
  }

  public static String createUrlPattern(String serviceId) {
    return "/smsc/esme_" + serviceId + "/*";
  }

}
