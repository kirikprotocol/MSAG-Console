package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.XmlUtils;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import javax.xml.parsers.DocumentBuilderFactory;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collection;
import java.util.LinkedList;
import java.util.Map;
import java.util.TimeZone;

/**
 * @author Aleksandr Khalitov
 */
class RegionsConfig implements ManagedConfigFile<RegionsSettings> {

  public void save(InputStream oldFile, OutputStream newFile, RegionsSettings conf) throws Exception {
    Map<Integer, Region> newRs = conf.getRegionsMap();
    Document d = XmlUtils.parse(oldFile);
    Element rootElement = d.getDocumentElement();
    NodeList regions = rootElement.getElementsByTagName("region");
    
    Collection<Region> removed = new LinkedList<Region>();
    for (int i = 0; i < regions.getLength(); i++) {
      Element region = (Element) regions.item(i);
      int id = Integer.parseInt(region.getAttribute("id"));
      if(!newRs.containsKey(id)) {
        Region r = new Region();
        r.setRegionId(id);
        readRegion(r, region);
        removed.add(r);
      }
    }

    d = DocumentBuilderFactory.newInstance().newDocumentBuilder().newDocument();
    rootElement = d.createElement("regions");
    d.appendChild(rootElement);

    for (Region r : newRs.values()) {
      Element region = d.createElement("region");
      fillElement(d, region, r);
      rootElement.appendChild(region);
    }

    for(Region r : removed) {
      Element region = d.createElement("region");
      fillElement(d, region, r);
      region.setAttribute("deleted", Boolean.toString(true));
      rootElement.appendChild(region);
    }

    Element defMax = d.createElement("region_default");
    rootElement.appendChild(defMax);
    defMax.setAttribute("bandwidth", Integer.toString(conf.getDefaultMaxPerSecond()));
    rootElement.appendChild(defMax);

    XmlUtils.storeConfig(newFile, d, System.getProperty("file.encoding"), "regions.dtd");
  }

  private static void fillElement(Document d, Element region, Region r) {
    region.setAttribute("id", Integer.toString(r.getRegionId()));
    region.setAttribute("name", r.getName());
    region.setAttribute("timezone", new StringBuilder(50).
        append(r.getTimeZone().getRawOffset() / 1000).append(',').append(r.getTimeZone().getID()).toString());
    region.setAttribute("infosme_smsc", r.getSmsc());
    region.setAttribute("bandwidth", Integer.toString(r.getMaxSmsPerSecond()));
    for (Address a : r.getMasks()) {
      Element mask = d.createElement("mask");
      mask.setAttribute("value", a.getSimpleAddress());
      region.appendChild(mask);
    }
  }

  private static void readRegion(Region r, Element region) throws AdminException {
    r.setName(region.getAttribute("name"));
    r.setMaxSmsPerSecond(Integer.parseInt(region.getAttribute("bandwidth")));
    r.setSmsc(region.getAttribute("infosme_smsc"));
    String tz = region.getAttribute("timezone");
    tz = tz.substring(tz.indexOf(',') + 1);
    r.setTimeZone(TimeZone.getTimeZone(tz));
    NodeList masks = region.getElementsByTagName("mask");
    for (int j = 0; j < masks.getLength(); j++) {
      Element mask = (Element) masks.item(j);
      r.addMask(new Address(mask.getAttribute("value")));
    }
  }

  public RegionsSettings load(InputStream is) throws Exception {
    Collection<Region> result = new LinkedList<Region>();
    Document d = XmlUtils.parse(is);
    Element rootElement = d.getDocumentElement();
    NodeList regions = rootElement.getElementsByTagName("region");
    int lastId = 1;
    for (int i = 0; i < regions.getLength(); i++) {
      Element region = (Element) regions.item(i);
      int id = Integer.parseInt(region.getAttribute("id"));
      if(id>lastId) {
        lastId = id;
      }
      String deleted = region.getAttribute("deleted");
      if(Boolean.valueOf(deleted)) {
        continue;
      }
      Region r = new Region();
      r.setRegionId(id);
      readRegion(r, region);
      result.add(r);
    }
    NodeList defs = d.getElementsByTagName("region_default");
    Element def = (Element) defs.item(0);
    int defaultMaxPerSecond = Integer.parseInt(def.getAttribute("bandwidth"));
    return new RegionsSettings(result, defaultMaxPerSecond, lastId);
  }
}
