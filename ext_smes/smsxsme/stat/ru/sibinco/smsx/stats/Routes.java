package ru.sibinco.smsx.stats;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.*;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
class Routes {


  private static Document parse(File f) throws Exception {
    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
    factory.setFeature("http://xml.org/sax/features/validation", false);
    DocumentBuilder builder = factory.newDocumentBuilder();
    return builder.parse(f);
  }

  static Collection<Region> parseRegions(File routes) throws Exception {

    Document regionsDoc =  parse(routes);

    Element listElement = regionsDoc.getDocumentElement();

    NodeList subjList = listElement.getElementsByTagName("subject_def");

    Collection<Region> result = new LinkedList<Region>();

    for (int i = 0; i < subjList.getLength(); i++) {
      Element subjElem = (Element) subjList.item(i);
      String name = subjElem.getAttribute("id");

      NodeList masksList = subjElem.getElementsByTagName("mask");
      Collection<String> masks = new ArrayList<String>(masksList.getLength());
      for (int j = 0; j < masksList.getLength(); j++) {
        Element maskElem = (Element) masksList.item(j);
        masks.add(maskElem.getAttribute("value").trim());
      }

      Region s = new Region();
      s.setId(name);
      s.addMasks(masks);
      result.add(s);
    }
    return result;
  }

  static Set<String> parseRoutes(File f) throws Exception {
    Set<String> result = new HashSet<String>();
    BufferedReader r = null;
    try{
      r = new BufferedReader(new InputStreamReader(new FileInputStream(f), "cp1251"));
      String line;
      while((line = r.readLine()) != null) {
        if(line.isEmpty()) {
          continue;
        }
        result.add(line.trim());
      }
      return result;
    }finally {
      if(r != null) {
        try{
          r.close();
        }catch (IOException e){}
      }
    }
  }

  /**
   * @author Aleksandr Khalitov
   */
  static class Region {

    private String id;

    private Set<String> masks = new HashSet<String>();

    public String getId() {
      return id;
    }

    public void setId(String id) {
      this.id = id;
    }

    public Set<String> getMasks() {
      return new HashSet<String>(masks);
    }

    public void addMasks(Collection<String> masks) {
      this.masks.addAll(masks);
    }

  }
}
