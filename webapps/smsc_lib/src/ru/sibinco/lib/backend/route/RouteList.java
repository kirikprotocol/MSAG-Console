/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 16:00:21
 */
package ru.sibinco.lib.backend.route;


/**
 * todo: remove it
 * @see ru.sibinco.lib.backend.util.SortedList
 * @see ru.sibinco.lib.backend.util.SortByPropertyComparator
 */
/*public class RouteList
{
  private Map map = new HashMap();

  public RouteList()
  {
  }

  public RouteList(Element routeListElement, SubjectList subjects, SmeManager smeManager)
      throws SibincoException
  {
    NodeList routeList = routeListElement.getElementsByTagName("route");
    for (int i = 0; i < routeList.getLength(); i++) {
      Element routeElem = (Element) routeList.item(i);
      put(new Route(routeElem, subjects, smeManager));
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
    Route r = (Route) map.remove(routeName);
    return r;
  }

  public void rename(String oldRouteName, String newRouteName)
  {
    Route r = remove(oldRouteName);
    r.setName(newRouteName);
    put(r);
  }

  public PrintWriter store(PrintWriter out)
  {
    for (Iterator i = iterator(); i.hasNext();) {
      ((Route) i.next()).store(out);
    }
    return out;
  }

  public boolean contains(String routeId)
  {
    return map.containsKey(routeId);
  }

  public boolean isSubjectUsed(String subjectId)
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
*/