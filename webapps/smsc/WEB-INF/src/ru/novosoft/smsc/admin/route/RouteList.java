/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 16:00:21
 */
package ru.novosoft.smsc.admin.route;

import java.util.Vector;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;


public class RouteList
{
  private Map map = new HashMap();

  public RouteList() {
  }

  public void put(Route r)
  {
    if (map.containsKey(r.getName()))
      throw new IllegalArgumentException("Route \"" + r.getName() + "\" already exist");

    map.put(r.getName(), r);
  }

  public Route get(String routeName)
  {
    return (Route) map.get(routeName);
  }

  public boolean isEmpty()
  {
    return map.isEmpty();
  }

  public Iterator iterator()
  {
    return map.values().iterator();
  }

  public Route remove(String routeName)
  {
    return (Route) map.remove(routeName);
  }

  public void rename(String oldRouteName, String newRouteName)
  {
    Route r = remove(oldRouteName);
    r.setName(newRouteName);
    put(r);
  }
}
