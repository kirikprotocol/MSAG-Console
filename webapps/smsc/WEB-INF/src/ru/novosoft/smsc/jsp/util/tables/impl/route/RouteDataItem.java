/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 16:43:51
 */
package ru.novosoft.smsc.jsp.util.tables.impl.route;

import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;


public class RouteDataItem extends AbstractDataItem
{
  protected RouteDataItem(Route r)
  {
    values.put("Route ID", r.getName());
    values.put("sources", r.getSources());
    values.put("destinations", r.getDestinations());
    values.put("isEnabling" , new Boolean(r.isEnabling()));
    values.put("isBilling"  , new Boolean(r.isBilling()));
    values.put("isArchiving", new Boolean(r.isArchiving()));
  }
}
