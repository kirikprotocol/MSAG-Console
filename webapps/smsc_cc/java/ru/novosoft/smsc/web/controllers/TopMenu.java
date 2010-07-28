package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.context.FacesContext;
import java.io.Serializable;
import java.security.Principal;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * author: alkhal
 */
public class TopMenu {

  private Collection<MenuBarItem> menuBarItems = new ArrayList<MenuBarItem>(4) {
    {
      add(new MenuBarItem("smsc.smsc", 133).
          add(new MenuItem("smsc.smsc.config", "/smsc/smsc/smsc_config.faces")).
          add(new MenuItem("smsc.smsc.reschedule", "/smsc/smsc/reschedule/index.faces")).
          add(new MenuItem("smsc.smsc.status", "/smsc/smsc/smsc_service/status.faces")).
          add(new MenuItem("smsc.smsc.users", "/smsc/smsc/users/index.faces")).
          add(new MenuItem("smsc.smsc.journal", "/smsc/smsc/journal/index.faces")).
          add(new MenuItem("smsc.smsc.regions", "/smsc/smsc/regions/regions.faces"))
      );
      add(new MenuBarItem("smsc.routes", 154).
          add(new MenuItem("smsc.routes.subjects", "/smsc/smsc/subjects/index.faces"))
      );
      add(new MenuBarItem("smsc.services", 63).
          add(new MenuItem("smsc.services.hosts", "/smsc/smsc/hosts/index.faces")).
          add(new MenuItem("smsc.services.services", "/smsc/smsc/services/index.faces"))
      );
      add(new MenuBarItem("smsc.help", 196).
          add(new MenuItem("smsc.help.manual", "/smsc/smsc/admin_guide.faces"))
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
      Iterator<MenuItem> iter = mbi.items.iterator();
      while (iter.hasNext()) {
        String url = iter.next().getRelativeUrl();
        Collection<String> roles = webXml.getRoles(url);
        if (roles == null) {
          continue;
        }
        if (roles.isEmpty()) {    // url is not accessable for all users
          iter.remove();
          break;
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
          break;
        }
      }
      if (mbi.items.isEmpty()) {  //remove itemBar if it's empty
        mbiIter.remove();
      }
    }
    return menuBarItems;
  }

  public static class MenuBarItem implements Serializable {

    public Collection<MenuItem> items = new LinkedList<MenuItem>();

    private String name;

    private int width;

    public MenuBarItem(String name, int width) {
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

    public int getWidth() {
      return width;
    }
  }


  public static class MenuItem implements Serializable {
    private String name;
    private String url;

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

    private String getRelativeUrl() {
      return url.substring(5);
    }
  }


}
