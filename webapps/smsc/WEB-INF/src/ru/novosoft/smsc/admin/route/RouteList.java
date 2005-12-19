package ru.novosoft.smsc.admin.route;

/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 16:00:21
 */

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.route.RouteDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.route.RouteQuery;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;


public class RouteList
{
  private Map map = new HashMap();
  private RouteDataSource dataSource = new RouteDataSource();

  public RouteList()
  {
  }

  public RouteList(Element routeListElement, SubjectList subjects, SmeManager smeManager)
          throws AdminException
  {
    NodeList routeList = routeListElement.getElementsByTagName("route");
    for (int i = 0; i < routeList.getLength(); i++) {
      Element routeElem = (Element) routeList.item(i);
      put(new Route(routeElem, subjects, smeManager));
    }
  }

  public synchronized void put(Route r)
  {
    if (map.containsKey(r.getName()))
      throw new IllegalArgumentException("Route \"" + r.getName() + "\" already exist");

    dataSource.add(r);
    map.put(r.getName(), r);
  }

  public synchronized Route get(String routeName)
  {
    return (Route) map.get(routeName);
  }

  public synchronized boolean isEmpty()
  {
    return map.isEmpty();
  }

  public synchronized Iterator iterator()
  {
    return map.values().iterator();
  }

  public synchronized Route remove(String routeName)
  {
    Route r = (Route) map.remove(routeName);
    if (r != null)
      dataSource.remove(r);
    return r;
  }

  public synchronized void rename(String oldRouteName, String newRouteName)
  {
    Route r = remove(oldRouteName);
    r.setName(newRouteName);
    put(r);
  }

  public synchronized PrintWriter store(PrintWriter out)
  {
    for (Iterator i = iterator(); i.hasNext();) {
      ((Route) i.next()).store(out);
    }
    return out;
  }

  public synchronized QueryResultSet query(RouteQuery query)
  {
    dataSource.clear();
    for (Iterator i = map.values().iterator(); i.hasNext();) {
      Route route = (Route) i.next();
      dataSource.add(route);
    }
    return dataSource.query(query);
  }

  public synchronized boolean contains(String routeId)
  {
    return map.containsKey(routeId);
  }

  public synchronized boolean isSubjectUsed(String subjectId)
  {
    for (Iterator i = map.values().iterator(); i.hasNext();) {
      Route route = (Route) i.next();
      if (route.getSources().isSubjectUsed(subjectId))
        return true;
      if (route.getDestinations().isSubjectUsed(subjectId))
        return true;
    }
    return false;
  }
}
