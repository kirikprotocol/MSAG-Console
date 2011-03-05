package ru.novosoft.smsc.admin.network_profiles;

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
 * @author Aleksandr Khalitov
 */
public class NetworkProfilesManager {

  private static final Category log = Category.getInstance(NetworkProfilesManager.class);
  private static NetworkProfilesManager instance = null;

  private final File profilesFile;
  private final HashMap profiles = new HashMap();
  private String defaultUssdOpenDestRef;
  private String defaultAbonentStatusMethod;

  private boolean modified = false;

  public static NetworkProfilesManager getInstance() throws AdminException {
    if (instance == null)
      instance = new NetworkProfilesManager();
    return instance;
  }


  private NetworkProfilesManager() throws AdminException {
    try {
      profilesFile = new File(WebAppFolders.getSmscConfFolder(), "network-profiles.xml");
      load();
    } catch (Exception e) {
      log.error(e,e);
      throw new AdminException("Can't init network profiles manager. Reason: " + e.getMessage());
    }
  }

  public synchronized void addProfile(NetworkProfile profile) throws AdminException {
    if(profiles.get(profile.getName()) != null) {
      throw new AdminException("Profile with name='"+profile.getName()+"' already exists");
    }
    saveProfile(profile);
  }

  public synchronized void editProfile(NetworkProfile profile) throws AdminException {
    saveProfile(profile);
  }

  private void saveProfile(NetworkProfile profile) throws AdminException {
    if(profile.getAbonentStatusMethod() == null || profile.getAbonentStatusMethod().length() == 0) {
      throw new AdminException("Abonent status method is empty");
    }
    if(profile.getMasks() == null || profile.getMasks().isEmpty()) {
      throw new AdminException("List of masks is empty");
    }

    Map masks = profile.getMasks();
    Iterator pI = profiles.values().iterator();
    while(pI.hasNext()) {
      NetworkProfile p = (NetworkProfile)pI.next();
      if(!p.getName().equals(profile.getName())) {
        Iterator mI = p.getMasks().values().iterator();
        while(mI.hasNext()) {
          String mask = ((Mask)mI.next()).getMaskSimple();
          if(masks.containsKey(mask)) {
            throw new AdminException("Masks intersection: "+mask);
          }
        }
      }
    }
    if(profile.getUssdOpenDestRef() == null) {
      profile.setUssdOpenDestRef("");
    }
    profiles.put(profile.getName(), profile);
    modified = true;
  }

  public synchronized void removeProfile(String name) throws AdminException {
    profiles.remove(name);
    modified = true;
  }

  public synchronized NetworkProfile getProfile(String name) throws AdminException {
    NetworkProfile n = (NetworkProfile) profiles.get(name);
    return n == null ? n : new NetworkProfile(n);
  }

  private synchronized void load() throws AdminException {
    profiles.clear();
    try {
      Document networkDoc = Utils.parse(profilesFile.getAbsolutePath());
      NodeList networks = networkDoc.getDocumentElement().getElementsByTagName("network");
      for (int i=0; i < networks.getLength(); i++) {
        Element nE = (Element)networks.item(i);
        final NetworkProfile network = new NetworkProfile(nE.getAttribute("name"));
        NodeList paramsList = nE.getElementsByTagName("param");
        for(int u=0;u<paramsList.getLength();u++) {
          Element pE = (Element)paramsList.item(u);
          if("ussdOpenDestRef".equals(pE.getAttribute("name"))) {
            network.setUssdOpenDestRef(pE.getAttribute("value"));
          }else if("abonentStatusMethod".equals(pE.getAttribute("name"))) {
            network.setAbonentStatusMethod(pE.getAttribute("value"));
          }
        }

        NodeList masksList = nE.getElementsByTagName("mask");
        Map masks = new HashMap(masksList.getLength());
        for(int y=0;y<masksList.getLength();y++) {
          String value = ((Element)masksList.item(y)).getAttribute("value");
          masks.put(value, new Mask(value));
        }
        network.setMasks(masks);
        this.profiles.put(network.getName(), network);
      }

      NodeList defs = networkDoc.getElementsByTagName("default");
      if (defs.getLength() > 0) {
        Element def = (Element)defs.item(0);
        NodeList paramsList = def.getElementsByTagName("param");
        for(int u=0;u<paramsList.getLength();u++) {
          Element pE = (Element)paramsList.item(u);
          if("ussdOpenDestRef".equals(pE.getAttribute("name"))) {
            defaultUssdOpenDestRef = pE.getAttribute("value");
          }else if("abonentStatusMethod".equals(pE.getAttribute("name"))) {
            defaultAbonentStatusMethod = pE.getAttribute("value");
          }
        }
      }

      modified = false;
    } catch (Exception e) {
      log.error(e,e);
      throw new AdminException("Can't load profiles. Reason: " + e.getMessage());
    }
  }

  private synchronized void save(File file) throws AdminException {
    PrintWriter out = null;

    try {
      out = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file), Functions.getLocaleEncoding())));
      Functions.storeConfigHeader(out, "networks", "network-profiles.dtd", Functions.getLocaleEncoding());
      for (Iterator iter = profiles.values().iterator(); iter.hasNext();) {
        NetworkProfile profile = (NetworkProfile)iter.next();
        out.print(" <network name=\"");out.print(profile.getName());out.println("\">");
        out.println("   <params>");
        out.print("     <param name=\"abonentStatusMethod\" value=\"");out.print(profile.getAbonentStatusMethod());out.println("\"/>");
        out.print("     <param name=\"ussdOpenDestRef\" value=\"");out.print(profile.getUssdOpenDestRef());out.println("\"/>");
        out.println("   </params>");
        out.println("   <masks>");
        for (Iterator masks = profile.getMasks().values().iterator(); masks.hasNext();) {
          Mask m = (Mask)masks.next();
          out.print("     <mask value=\"");out.print(m.getMaskSimple());out.println("\"/>");
        }
        out.println("   </masks>");
        out.println(" </network>");
      }
      out.println(" <default>");
      out.println("   <params>");
      out.print("     <param name=\"abonentStatusMethod\" value=\"");out.print(defaultAbonentStatusMethod);out.println("\"/>");
      out.print("     <param name=\"ussdOpenDestRef\" value=\"");if(defaultUssdOpenDestRef != null)out.print(defaultUssdOpenDestRef);out.println("\"/>");
      out.println("   </params>");
      out.println(" </default>");

      Functions.storeConfigFooter(out, "networks");
      modified = false;
    } catch (IOException e) {
      log.error(e,e);
      throw new AdminException("Can't save profiles. Reason: " + e.getMessage());
    } finally {
      if (out != null)
        out.close();
    }
  }

  public synchronized void save() throws AdminException {

    try {
      final File newFile = Functions.createNewFilenameForSave(profilesFile);
      save(newFile);
      Functions.renameNewSavedFileToOriginal(newFile, profilesFile);
    } catch (IOException e) {
      log.error(e,e);
      throw new AdminException("Can't save profiles. Reason: " + e.getMessage());
    }
  }

  public synchronized void reset() throws AdminException {
    load();
  }

  public synchronized Collection getProfiles() throws AdminException {
    Collection results = new ArrayList(profiles.size());
    Iterator i = profiles.values().iterator();
    while(i.hasNext()) {
      results.add(new NetworkProfile((NetworkProfile)i.next()));
    }
    return results;
  }

  public String getDefaultUssdOpenDestRef() {
    return defaultUssdOpenDestRef;
  }

  public void setDefaultUssdOpenDestRef(String defaultUssdOpenDestRef) {
    modified = modified || (defaultUssdOpenDestRef != null &&  !defaultUssdOpenDestRef.equals(this.defaultUssdOpenDestRef)) ||
        (this.defaultUssdOpenDestRef != null &&  !this.defaultUssdOpenDestRef.equals(defaultUssdOpenDestRef));
    this.defaultUssdOpenDestRef = defaultUssdOpenDestRef;

  }

  public String getDefaultAbonentStatusMethod() {
    return defaultAbonentStatusMethod;
  }

  public void setDefaultAbonentStatusMethod(String defaultAbonentStatusMethod) {
    modified = modified || (defaultAbonentStatusMethod != null &&  !defaultAbonentStatusMethod.equals(this.defaultAbonentStatusMethod)) ||
        (this.defaultAbonentStatusMethod != null &&  !this.defaultAbonentStatusMethod.equals(defaultAbonentStatusMethod));
    this.defaultAbonentStatusMethod = defaultAbonentStatusMethod;
  }

  protected void setModified() {
    modified = true;
  }

  public boolean isModified() {
    return modified;
  }


}
