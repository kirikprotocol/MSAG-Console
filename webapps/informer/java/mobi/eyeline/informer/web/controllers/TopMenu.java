package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.WebContext;
import mobi.eyeline.informer.util.xml.WebXml;

import javax.faces.context.FacesContext;
import java.io.Serializable;
import java.security.Principal;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * Контроллер для отображения главного меню
 * @author Aleksandr Khalitov
 */
public class TopMenu {

  private final Collection<MenuBarItem> menuBarItems = new ArrayList<MenuBarItem>(3) {
    {
      add(new MenuBarItem("informer.admin", "informer.admin.width").
          add(new MenuItem("informer.admin.status", "/status.faces")).
          add(new MenuItem("informer.admin.config", "/config/index.faces")).
          add(new MenuItem("informer.admin.users", "/users/index.faces")).
          add(new MenuItem("informer.admin.regions", "/regions/index.faces")).
          add(new MenuItem("informer.admin.smsc", "/smsc/index.faces")).
          add(new MenuItem("informer.admin.blacklist", "/blacklist/index.faces")).
          add(new MenuItem("informer.admin.reschedules", "/reschedule/index.faces")).
          add(new MenuItem("informer.admin.prohibitions", "/prohibitions/index.faces")).
          add(new MenuItem("informer.admin.siebel", "/siebel/index.faces")).
          add(new MenuItem("informer.admin.import", "/import/index.faces")).
          add(new MenuItem("informer.admin.archieve", "/archieve/index.faces")).
          add(new MenuItem("informer.admin.cdr", "/cdr/index.faces")).
          add(new MenuItem("informer.admin.journal", "/journal/index.faces"))
      );
      add(new MenuBarItem("informer.deliveries", "informer.deliveries.width").
          add(new MenuItem("informer.deliveries.list", "/deliveries/index.faces")).
          add(new MenuItem("informer.deliveries.scheduling", "/deliveries/scheduling.faces")).
          add(new MenuItem("informer.deliveries.create", "/deliveries/create.faces"))
      );
      add(new MenuBarItem("informer.stats", "informer.stats.width").
          add(new MenuItem("informer.stats.common", "/stats/common.faces")).
          add(new MenuItem("informer.stats.messages", "/stats/messages.faces")).
          add(new MenuItem("informer.stats.deliveries", "/stats/deliveries.faces")).
          add(new MenuItem("informer.stats.messages.recipients", "/stats/messagesByRecs.faces")).
          add(new MenuItem("informer.stats.messages.deliveries", "/stats/messagesByDels.faces"))
      );
    }
  };

  public Collection<MenuBarItem> getMenuBarItems() {
    Principal p = FacesContext.getCurrentInstance().getExternalContext().getUserPrincipal();
    if (p == null) {
      return null;
    }
    return getByUser(p.getName());
  }

  private Collection<MenuBarItem> getByUser(String userName) {
    WebXml webXml = WebContext.getInstance().getWebXml();
    Iterator<MenuBarItem> mbiIter = menuBarItems.iterator();
    while (mbiIter.hasNext()) {
      MenuBarItem mbi = mbiIter.next();
//      System.out.println("Bar item: "+mbi.name);
      Iterator<MenuItem> iter = mbi.items.iterator();
      while (iter.hasNext()) {
        String url = iter.next().getUrl();
        Collection<String> roles = webXml.getRoles(url);
//        System.out.println("Needed roles: "+roles+" for "+url);
        if (roles == null) {      // url is accessable for all
          continue;
        }
        if (roles.isEmpty()) {    // url is not accessable for all users
          iter.remove();
          continue;
        }
        boolean access = false;
        for (String role : roles) {
          if (FacesContext.getCurrentInstance().getExternalContext().isUserInRole(role)) {
            access = true;
            break;
          }
        }
        if (!access) {   // user isn't in roles group
          iter.remove();
        }
      }
      if (mbi.items.isEmpty()) {  //remove itemBar if it's empty
        mbiIter.remove();
      }
    }
    return menuBarItems;
  }

  public static class MenuBarItem implements Serializable {

    public final Collection<MenuItem> items = new LinkedList<MenuItem>();

    private final String name;

    private final String width;

    public MenuBarItem(String name, String width) {
      this.name = name;
      this.width = width;
    }

    public Collection<MenuItem> getItems() {
      return items;
    }

    public MenuBarItem add(MenuItem mi) {
      items.add(mi);
      return this;
    }

    public boolean removeItem(MenuItem mi) {
      return items.remove(mi);
    }

    public String getName() {
      return name;
    }

    public String getWidth() {
      return width;
    }
  }


  public static class MenuItem implements Serializable {
    private final String name;
    private final String url;

    public MenuItem(String name, String url) {
      this.name = name;
      this.url = url;
    }

    public String getName() {
      return name;
    }

    public String getUrl() {
      return url;
    }
  }


}
