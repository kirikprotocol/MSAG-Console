package ru.novosoft.smsc.admin.smsstat;

/*
 Created by andrey Date: 07.02.2005 Time: 12:45:18
 */

//import org.w3c.dom.Element;
//import org.w3c.dom.NodeList;
//import ru.novosoft.smsc.admin.AdminException;
//import ru.novosoft.smsc.admin.route.SubjectList;

import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.smcstat.StatRouteDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.smcstat.StatRouteQuery;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;


public class StatRouteList
{
  private Map map = new HashMap();
  private StatRouteDataSource dataSource = new StatRouteDataSource();

  public StatRouteList()
  {
  }

  /* public StatRouteList(Element routeListElement, SubjectList subjects, SmeManager smeManager)
           throws AdminException
   {
     NodeList routeList = routeListElement.getElementsByTagName("route");
     for (int i = 0; i < routeList.getLength(); i++) {
       Element routeElem = (Element) routeList.item(i);
       put(new Route(routeElem, subjects, smeManager));
     }
   }
    */
  public void put(RouteIdCountersSet r)
  {
    if (map.containsKey(r.routeid))
      throw new IllegalArgumentException("StatRoute \"" + r.routeid + "\" already exist");

    dataSource.add(r);
    map.put(r.routeid, r);
  }

  public RouteIdCountersSet get(String routeName)
  {
    return (RouteIdCountersSet) map.get(routeName);
  }

  public boolean isEmpty()
  {
    return map.isEmpty();
  }

  public Iterator iterator()
  {
    return map.values().iterator();
  }

  public RouteIdCountersSet remove(String routeName)
  {
    RouteIdCountersSet r = (RouteIdCountersSet) map.remove(routeName);
    if (r != null)
      dataSource.remove(r);
    return r;
  }

  /*
   public void rename(String oldRouteName, String newRouteName)
   {
     RouteIdCountersSet r = remove(oldRouteName);
     r.setName(newRouteName);
     put(r);
   }

   public PrintWriter store(PrintWriter out)
   {
     for (Iterator i = iterator(); i.hasNext();) {
       ((RouteIdCountersSet) i.next()).store(out);
     }
     return out;
   }
    */
  public QueryResultSet query(StatRouteQuery query)
  {
    dataSource.clear();
    for (Iterator i = map.values().iterator(); i.hasNext();) {
      RouteIdCountersSet r = (RouteIdCountersSet) i.next();
      dataSource.add(r);
    }
    return dataSource.query(query);
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
