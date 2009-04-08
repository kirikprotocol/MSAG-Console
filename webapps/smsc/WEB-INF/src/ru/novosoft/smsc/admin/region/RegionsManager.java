package ru.novosoft.smsc.admin.region;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.xml.Utils;

import java.io.*;
import java.util.*;

/**
 * User: artem
 * Date: Aug 1, 2007
 */

public class RegionsManager {

  private static final Category log = Category.getInstance(RegionsManager.class);
  private static RegionsManager instance = null;

  private final File regionsFile;
  private final HashMap regions = new HashMap();
  private int defaultBandwidth;
  private String defaultEmail;
  private int id = 0;

  private boolean modified = false;

  public static RegionsManager getInstance() throws AdminException {
    if (instance == null)
      instance = new RegionsManager();
    return instance;
  }


  private RegionsManager() throws AdminException {
    try {
      regionsFile = new File(WebAppFolders.getSmscConfFolder(), "regions.xml");
      load();
    } catch (Exception e) {
      log.error(e,e);
      throw new AdminException("Can't init regions manager. Reason: " + e.getMessage());
    }
  }

  public synchronized void addRegion(final Region region) throws AdminException {
    if (!region.isExists()) {
      id++;
      region.setId(id);
    }

    regions.put(new Integer(region.getId()), region);

    modified = true;
  }

  public synchronized void removeRegion(int id) throws AdminException {
    try {
      regions.remove(new Integer(id));
      modified = true;
    } catch (Exception e) {
      log.error(e,e);
      throw new AdminException("Can't add region. Reason: " + e.getMessage());
    }
  }

  public synchronized Region getRegionById(int id) throws AdminException {
    return (Region)regions.get(new Integer(id));
  }

  private synchronized void load() throws AdminException {
    regions.clear();
    try {
      Document regionsDoc = Utils.parse(regionsFile.getAbsolutePath());
      NodeList regions = regionsDoc.getDocumentElement().getElementsByTagName("region");
      for (int i=0; i < regions.getLength(); i++) {
        Element el = (Element)regions.item(i);
        final Region region = new Region(el.getAttribute("name"));
        region.setId(Integer.parseInt(el.getAttribute("id")));
        region.setBandWidth(Integer.parseInt(el.getAttribute("bandwidth")));
        region.setEmail(el.getAttribute("email"));

        TimeZone tz = null;
        String tzId = el.getAttribute("timezone");
        if (tzId != null)
          tz = TimeZone.getTimeZone(tzId);
        if (tz == null)
          tz = TimeZone.getDefault();
        region.setTimezone(tz);

        NodeList subjects = el.getElementsByTagName("subject");
        for (int j=0; j < subjects.getLength(); j++) {
          region.addSubject((((Element)subjects.item(j)).getAttribute("id")));
        }

        id = Math.max(id, region.getId());
        this.regions.put(new Integer(region.getId()), region);
      }

      NodeList defRegions = regionsDoc.getElementsByTagName("region_default");
      if (defRegions.getLength() > 0) {
        Element defRegion = (Element)defRegions.item(0);
        defaultBandwidth = Integer.parseInt(defRegion.getAttribute("bandwidth"));
        defaultEmail = defRegion.getAttribute("email");
      }

      modified = false;
    } catch (Exception e) {
      log.error(e,e);
      throw new AdminException("Can't add region. Reason: " + e.getMessage());
    }
  }

  private synchronized void check() throws AdminException {
    if (defaultBandwidth <= 0)
      throw new AdminException("Invalid default bandwidth: " + defaultBandwidth);
    Region region1, region2;

    for (Iterator iter1 = regions.values().iterator(); iter1.hasNext();) {
      region1 = (Region)iter1.next();
      for (Iterator iter2 = regions.values().iterator(); iter2.hasNext();) {
        region2 = (Region)iter2.next();
        if (region1 != region2)
          for (Iterator subjects = region2.getSubjects().iterator(); subjects.hasNext();) {
            String subject = (String)subjects.next();
            if (region1.getSubjects().contains(subject))
              throw new AdminException("Regions " + region1.getName() + " and " + region2.getName() + " contain the same subject " + subject);
          }
      }
    }
  }

  private synchronized void save(File file) throws AdminException {
    PrintWriter out = null;

    try {
      out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(file), Functions.getLocaleEncoding()));
      Functions.storeConfigHeader(out, "regions", "regions.dtd", Functions.getLocaleEncoding());
      for (Iterator iter = regions.entrySet().iterator(); iter.hasNext();) {
        Map.Entry entry = (Map.Entry)iter.next();
        Region region = (Region)entry.getValue();
        out.println("  <region id=\"" + region.getId() + "\" name=\"" + region.getName() +
                                                         "\" bandwidth=\"" + region.getBandWidth() + "\"" +
                                                        (region.getEmail()==null || region.getEmail().length() == 0 ? "" : " email=\"" + region.getEmail() + "\" ") +
                                                        (region.getTimezone() == null ? "" : " timezone=\"" + region.getTimezone().getID() + "\" ")+ ">");
        out.println("    <subjects>");
        for (Iterator subjects = region.getSubjects().iterator(); subjects.hasNext();)
          out.println("      <subject id=\"" + subjects.next() + "\"/>");
        out.println("    </subjects>");
        out.println("  </region>");
      }
      out.println("  <region_default bandwidth=\"" + defaultBandwidth + "\"" + (defaultEmail==null || defaultEmail.length() == 0 ? "" : " email=\"" + defaultEmail + "\" ") + "/>");
      Functions.storeConfigFooter(out, "regions");
      modified = false;
    } catch (IOException e) {
      log.error(e,e);
      throw new AdminException("Can't save regions. Reason: " + e.getMessage());
    } finally {
      if (out != null)
        out.close();
    }
  }

  public synchronized void save() throws AdminException {
    check();

    try {
      final File newFile = Functions.createNewFilenameForSave(regionsFile);
      save(newFile);
      Functions.renameNewSavedFileToOriginal(newFile, regionsFile);
    } catch (IOException e) {
      log.error(e,e);
      throw new AdminException("Can't save regions. Reason: " + e.getMessage());
    }
  }

  public synchronized void reset() throws AdminException {
    load();
  }

  public synchronized Collection getRegions() {
    return regions.values();
  }

  public synchronized int getDefaultBandwidth() {
    return defaultBandwidth;
  }

  public synchronized void setDefaultBandwidth(int defaultBandwidth) {
    modified = modified || defaultBandwidth != this.defaultBandwidth;
    this.defaultBandwidth = defaultBandwidth;
  }

  public synchronized String getDefaultEmail() {
    return defaultEmail;
  }

  public synchronized void setDefaultEmail(String defaultEmail) {
    modified = modified || !defaultEmail.equals(this.defaultEmail);      
    this.defaultEmail = defaultEmail;
  }

  protected void setModified() {
    modified = true;
  }

  public boolean isModified() {
    return modified;
  }
}
