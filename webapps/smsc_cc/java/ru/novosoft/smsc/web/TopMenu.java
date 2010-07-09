package ru.novosoft.smsc.web;

import java.io.Serializable;
import java.util.Collection;
import java.util.LinkedList;

/**
 * author: alkhal
 */
public class TopMenu {

  private Collection<MenuBarItem> menuBarItems = new LinkedList<MenuBarItem>()
  {
    {
      add(new MenuBarItem("SMSC", 133){{
        items.add(new MeniItem("Статус", "/smsc/smsc/smsc_service/status.jsp"));
        items.add(new MeniItem("Пользователи", "/smsc/smsc/users/index.jsp"));
        items.add(new MeniItem("Журнал", "/smsc/smsc/journal/index.jsp"));
        items.add(new MeniItem("Регионы", "/smsc/smsc/regions/regions.jsp"));
      }});
      add(new MenuBarItem("Маршрутизация", 154){{
        items.add(new MeniItem("Субъекты", "/smsc/smsc/subjects/index.jsp"));
      }});
      add(new MenuBarItem("Сервисы", 63){{
        items.add(new MeniItem("Хосты", "/smsc/smsc/hosts/index.jsp"));
        items.add(new MeniItem("Сервисы", "/smsc/smsc/services/index.jsp"));
      }});
      add(new MenuBarItem("Помощь", 196){{
        items.add(new MeniItem("Руководство администратора", "/smsc/admin_guide.jsp"));
      }});
    }
  };

  public Collection<MenuBarItem> getMenuBarItems() {
    return menuBarItems;
  }

  public static class MenuBarItem implements Serializable{

    public Collection<MeniItem> items = new LinkedList<MeniItem>();

    private String name;

    private int width;

    public MenuBarItem(String name, int width) {
      this.name = name;
      this.width = width;
    }

    public Collection<MeniItem> getItems() {
      return items;
    }

    public void addItem(MeniItem mi) {
      items.add(mi);
    }

    public boolean removeItem(MeniItem mi) {
      return items.remove(mi);
    }

    public String getName() {
      return name;
    }

    public int getWidth() {
      return width;
    }
  }


  public static class MeniItem implements Serializable{
    private String name;
    private String url;

    public MeniItem(String name, String url) {
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
