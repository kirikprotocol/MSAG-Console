/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 16:00:21
 */
package ru.novosoft.smsc.admin.route;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;


public class RouteList
{
  private Map map = new HashMap();

  public RouteList()
  {
  }

  public RouteList(Element routeListElement, SubjectList subjects, SMEList smes)
          throws AdminException
  {
    NodeList routeList = routeListElement.getElementsByTagName("route");
    for (int i = 0; i < routeList.getLength(); i++)
    {
      Element routeElem = (Element) routeList.item(i);
      put(new Route(routeElem, subjects, smes));
    }
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

  public PrintWriter store(PrintWriter out)
  {
    for (Iterator i = iterator(); i.hasNext();)
    {
      ((Route) i.next()).store(out);
    }
    return out;
  }
}
