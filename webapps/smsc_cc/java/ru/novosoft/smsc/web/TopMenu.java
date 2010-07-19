package ru.novosoft.smsc.web;

import ru.novosoft.smsc.util.xml.WebXml;

import javax.faces.context.FacesContext;
import java.io.Serializable;
import java.security.Principal;
import java.util.*;

/**
 * author: alkhal
 */
public class TopMenu {

  private Collection<MenuBarItem> menuBarItems = new ArrayList<MenuBarItem>(4) {
    {
      add(new MenuBarItem("SMSC", 133).
          add(new MenuItem("Статус", "/smsc/smsc/smsc_service/status.faces")).
          add(new MenuItem("Пользователи", "/smsc/smsc/users/index.faces")).
          add(new MenuItem("Журнал", "/smsc/smsc/journal/index.faces")).
          add(new MenuItem("Регионы", "/smsc/smsc/regions/regions.faces"))
      );
      add(new MenuBarItem("Маршрутизация", 154).
          add(new MenuItem("Субъекты", "/smsc/smsc/subjects/index.faces"))
      );
      add(new MenuBarItem("Сервисы", 63).
          add(new MenuItem("Хосты", "/smsc/smsc/hosts/index.faces")).
          add(new MenuItem("Сервисы", "/smsc/smsc/services/index.faces"))
      );
      add(new MenuBarItem("Помощь", 196).
          add(new MenuItem("Руководство администратора", "/smsc/smsc/admin_guide.faces"))
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
