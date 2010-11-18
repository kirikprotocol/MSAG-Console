package mobi.eyeline.informer.admin.regions;

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
import java.util.TimeZone;

/**
 * @author Aleksandr Khalitov
 */
class RegionsConfig implements ManagedConfigFile<RegionsSettings> {

  public void save(InputStream oldFile, OutputStream newFile, RegionsSettings conf) throws Exception {
    Document d = DocumentBuilderFactory.newInstance().newDocumentBuilder().newDocument();
    Element rootElement = d.createElement("regions");
    d.appendChild(rootElement);
    for (Region r : conf.getRegions()) {
      Element region = d.createElement("region");
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
      rootElement.appendChild(region);
    }

    Element defMax = d.createElement("region_default");
    defMax.setAttribute("bandwidth", Integer.toString(conf.getDefaultMaxPerSecond()));
    rootElement.appendChild(defMax);

    XmlUtils.storeConfig(newFile, d, System.getProperty("file.encoding"), "regions.dtd");
  }

  public RegionsSettings load(InputStream is) throws Exception {
    Collection<Region> result = new LinkedList<Region>();
    Document d = XmlUtils.parse(is);
    NodeList regions = d.getElementsByTagName("region");
    for (int i = 0; i < regions.getLength(); i++) {
      Region r = new Region();
      Element region = (Element) regions.item(i);
      r.setName(region.getAttribute("name"));
      r.setMaxSmsPerSecond(Integer.parseInt(region.getAttribute("bandwidth")));
      r.setSmsc(region.getAttribute("infosme_smsc"));
      String tz = region.getAttribute("timezone");
      tz = tz.substring(tz.indexOf(',') + 1);
      r.setTimeZone(TimeZone.getTimeZone(tz));
      r.setRegionId(Integer.parseInt(region.getAttribute("id")));
      NodeList masks = region.getElementsByTagName("mask");
      for (int j = 0; j < masks.getLength(); j++) {
        Element mask = (Element) masks.item(j);
        r.addMask(new Address(mask.getAttribute("value")));
      }
      result.add(r);
    }
    NodeList defs = d.getElementsByTagName("region_default");
    Element def = (Element) defs.item(0);
    int defaultMaxPerSecond = Integer.parseInt(def.getAttribute("bandwidth"));
    return new RegionsSettings(result, defaultMaxPerSecond);
  }
}
