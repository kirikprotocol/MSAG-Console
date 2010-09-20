package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.config.ManagedConfigFile;
import mobi.eyeline.informer.util.XmlUtils;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import javax.xml.parsers.DocumentBuilderFactory;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.*;

/**
 * author: alkhal
 */
class UsersConfig implements ManagedConfigFile<UsersSettings>{

  public void save(InputStream is, OutputStream os, UsersSettings settings) throws Exception {
    Document d = DocumentBuilderFactory.newInstance().newDocumentBuilder().newDocument();
    Element rootElement = d.createElement("users");
    d.appendChild(rootElement);
    for(User u : settings.getUsers())
      rootElement.appendChild(formatUser(u, d));
    XmlUtils.storeConfig(os, d, System.getProperty("file.encoding"), "users.dtd");
  }

  static Collection<Element> formatUserPrefs(UserPreferences prefs, Document d) {
    Collection<Element> result = new LinkedList<Element>();
    Element e = d.createElement("pref");
    e.setAttribute("name", "locale");
    e.setAttribute("value", prefs.getLocale() == null ? "" :prefs.getLocale().getLanguage());
    result.add(e);
    e = d.createElement("pref");
    e.setAttribute("name", "topmon.graph.head");
    e.setAttribute("value", Integer.toString(prefs.getTopMonGraphHead()));
    result.add(e);
    e = d.createElement("pref");
    e.setAttribute("name", "topmon.graph.scale");
    e.setAttribute("value", Integer.toString(prefs.getTopMonGraphScale()));
    result.add(e);
    e = d.createElement("pref");
    e.setAttribute("name", "topmon.max.speed");
    e.setAttribute("value", Integer.toString(prefs.getTopMonMaxSpeed()));
    result.add(e);
    e = d.createElement("pref");
    e.setAttribute("name", "topmon.graph.higrid");
    e.setAttribute("value", Integer.toString(prefs.getTopMonGraphHigrid()));
    result.add(e);
    e = d.createElement("pref");
    e.setAttribute("name", "topmon.graph.grid");
    e.setAttribute("value", Integer.toString(prefs.getTopMonGraphGrid()));
    result.add(e);
    e = d.createElement("pref");
    e.setAttribute("name", "perfmon.vMinuteGrid");
    e.setAttribute("value", Integer.toString(prefs.getPerfMonVMinuteGrid()));
    result.add(e);
    e = d.createElement("pref");
    e.setAttribute("name", "perfmon.vLightGrid");
    e.setAttribute("value", Integer.toString(prefs.getPerfMonVLightGrid()));
    result.add(e);
    e = d.createElement("pref");
    e.setAttribute("name", "perfmon.block");
    e.setAttribute("value", Integer.toString(prefs.getPerfMonBlock()));
    result.add(e);
    e = d.createElement("pref");
    e.setAttribute("name", "perfmon.pixPerSecond");
    e.setAttribute("value", Integer.toString(prefs.getPerfMonPixPerSecond()));
    result.add(e);
    e = d.createElement("pref");
    e.setAttribute("name", "perfmon.scale");
    e.setAttribute("value", Integer.toString(prefs.getPerfMonScale()));
    result.add(e);

    for(Map.Entry<String, String> entry : prefs.getUnknown().entrySet()) {
      e = d.createElement("pref");
      e.setAttribute("name", entry.getKey());
      e.setAttribute("value", entry.getValue() == null ? "" : entry.getValue());
      result.add(e);
    }
    return result;
  }

  static Element formatUser(User u, Document d) {
    Element e = d.createElement("user");
    e.setAttribute("login", u.getLogin() != null ? u.getLogin() : "");
    e.setAttribute("password", u.getPassword() != null ? u.getPassword() : "");
    e.setAttribute("firstName", u.getFirstName() != null ? u.getFirstName() : "");
    e.setAttribute("lastName", u.getLastName() != null ? u.getLastName() : "");
    e.setAttribute("dept", u.getDept() != null ? u.getDept() : "");
    e.setAttribute("workPhone", u.getWorkPhone() != null ? u.getWorkPhone() : "");
    e.setAttribute("homePhone", u.getHomePhone() != null ? u.getHomePhone() : "");
    e.setAttribute("cellPhone", u.getCellPhone() != null ? u.getCellPhone() : "");
    e.setAttribute("email", u.getEmail() != null ? u.getEmail() : "");

    for(String r : u.getRoles()) {
      Element c = d.createElement("role");
      c.setAttribute("name", r);
      e.appendChild(c);
    }

    for(Element p : formatUserPrefs(u.getPrefs(), d)) {
      e.appendChild(p);
    }

    return e;
  }

  public UsersSettings load(InputStream is) throws Exception {
    UsersSettings us = new UsersSettings();
    Document d = XmlUtils.parse(is);
    NodeList users = d.getElementsByTagName("user");
    List<User> result = new ArrayList<User>(users.getLength());
    for(int i=0;i<users.getLength();i++) {
      Element e = (Element)users.item(i);
      result.add(parseUser(e));
    }
    us.setUsers(result);
    return us;
  }

  static User parseUser(Element userElem) throws AdminException {
    User user = new User();
    user.setLogin(userElem.getAttribute("login"));
    user.setPassword(userElem.getAttribute("password"));

    Set<String> roles = new TreeSet<String>();
    NodeList roleList = userElem.getElementsByTagName("role");
    for (int i = 0; i < roleList.getLength(); i++) {
      Element roleElem = (Element) roleList.item(i);
      roles.add(roleElem.getAttribute("name"));
    }
    user.setRoles(roles);

    NodeList prefList = userElem.getElementsByTagName("pref");

    UserPreferences prefs = parsePrefs(prefList);

    user.setPrefs(prefs);

    user.setFirstName(userElem.getAttribute("firstName"));
    user.setLastName(userElem.getAttribute("lastName"));
    user.setDept(userElem.getAttribute("dept"));
    user.setWorkPhone(userElem.getAttribute("workPhone"));
    user.setHomePhone(userElem.getAttribute("homePhone"));
    user.setCellPhone(userElem.getAttribute("cellPhone"));
    user.setEmail(userElem.getAttribute("email"));
    return user;
  }

  static UserPreferences parsePrefs(NodeList prefsList) throws AdminException{
    UserPreferences prefs = new UserPreferences();

    Map<String, String> unknown = new HashMap<String, String>();

    for(int i=0; i<prefsList.getLength(); i++) {
      Element e = (Element)prefsList.item(i);
      String name = e.getAttribute("name");
      String value = e.getAttribute("value");
      if(name != null && value != null && value.length()>0) {
        if(name.equals("locale")){
          prefs.setLocale(new Locale(value));
        }else if(name.equals("topmon.graph.head")) {
          prefs.setTopMonGraphHead(Integer.parseInt(value));
        }else if(name.equals("topmon.graph.scale")) {
          prefs.setTopMonGraphScale(Integer.parseInt(value));
        }else if(name.equals("topmon.max.speed")) {
          prefs.setTopMonMaxSpeed(Integer.parseInt(value));
        }else if(name.equals("topmon.graph.higrid")) {
          prefs.setTopMonGraphHigrid(Integer.parseInt(value));
        }else if(name.equals("topmon.graph.grid")) {
          prefs.setTopMonGraphGrid(Integer.parseInt(value));
        }else if(name.equals("perfmon.vMinuteGrid")) {
          prefs.setPerfMonVMinuteGrid(Integer.parseInt(value));
        }else if(name.equals("perfmon.vLightGrid")) {
          prefs.setPerfMonVLightGrid(Integer.parseInt(value));
        }else if(name.equals("perfmon.block")) {
          prefs.setPerfMonBlock(Integer.parseInt(value));
        }else if(name.equals("perfmon.pixPerSecond")) {
          prefs.setPerfMonPixPerSecond(Integer.parseInt(value));
        }else if(name.equals("perfmon.scale")) {
          prefs.setPerfMonScale(Integer.parseInt(value));
        }else {
          unknown.put(name, value);
        }
      }
    }

    prefs.setUnknown(unknown);

    return prefs;
  }

}
