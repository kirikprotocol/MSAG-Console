package ru.novosoft.smsc.admin.region;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.XmlUtils;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.TimeZone;

/**
 * @author Artem Snopkov
 */
class RegionsConfig implements ManagedConfigFile<RegionSettings> {

  public void save(InputStream oldFile, OutputStream newFile, RegionSettings conf) throws Exception {
    PrintWriter out = null;
    try {
      out = new PrintWriter(new OutputStreamWriter(newFile, Functions.getLocaleEncoding()));
      XmlUtils.storeConfigHeader(out, "regions", "regions.dtd", Functions.getLocaleEncoding());
      for (Region region : conf.getRegions()) {
        out.println("  <region id=\"" + region.getId() + "\" name=\"" + region.getName() +
            "\" bandwidth=\"" + region.getBandwidth() + "\"" +
            (region.getEmail() == null || region.getEmail().length() == 0 ? "" : " email=\"" + region.getEmail() + "\" ") +
            (region.getTimezone() == null ? "" : " timezone=\"" + region.getTimezone().getID() + "\" ") + ">");
        out.println("    <subjects>");
        for (Object s : region.getSubjects())
          out.println("      <subject id=\"" + s + "\"/>");
        out.println("    </subjects>");
        out.println("  </region>");
      }
      out.println("  <region_default bandwidth=\"" + conf.getDefaultBandwidth() + "\"" + (conf.getDefaultEmail() == null || conf.getDefaultEmail().length() == 0 ? "" : " email=\"" + conf.getDefaultEmail() + "\" ") + "/>");
      XmlUtils.storeConfigFooter(out, "regions");
    } finally {
      if (out != null)
        out.close();
    }
  }

  public RegionSettings load(InputStream is) throws Exception {
    Collection<Region> regionsList = new ArrayList<Region>();
    Document regionsDoc = XmlUtils.parse(is);
    NodeList regions = regionsDoc.getDocumentElement().getElementsByTagName("region");
    for (int i = 0; i < regions.getLength(); i++) {
      Element el = (Element) regions.item(i);
      final Region region = new Region(Integer.parseInt(el.getAttribute("id")), el.getAttribute("name"));

      region.setBandwidth(Integer.parseInt(el.getAttribute("bandwidth")));
      region.setEmail(el.getAttribute("email"));

      TimeZone tz = null;
      String tzId = el.getAttribute("timezone");
      if (tzId != null)
        tz = TimeZone.getTimeZone(tzId);
      if (tz == null)
        tz = TimeZone.getDefault();
      region.setTimezone(tz);

      NodeList subjects = el.getElementsByTagName("subject");
      List<String> subjectsList = new ArrayList<String>();
      for (int j = 0; j < subjects.getLength(); j++) {
        subjectsList.add((((Element) subjects.item(j)).getAttribute("id")));
      }

      region.setSubjects(subjectsList);
      regionsList.add(region);
    }

    String defaultEmail = null;
    int defaultBandwidth = 0;
    NodeList defRegions = regionsDoc.getElementsByTagName("region_default");
    if (defRegions.getLength() > 0) {
      Element defRegion = (Element) defRegions.item(0);
      defaultBandwidth = Integer.parseInt(defRegion.getAttribute("bandwidth"));
      defaultEmail = defRegion.getAttribute("email");
      if (defaultEmail != null && defaultEmail.length() == 0)
        defaultEmail = null;
    }

    return new RegionSettings(regionsList, defaultBandwidth, defaultEmail);
  }
}
