package ru.novosoft.smsc.jsp.util.tables.impl.route;

/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 16:43:51
 */

import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;


public class RouteDataItem extends AbstractDataItem {
    protected RouteDataItem(Route r) {
        values.put("Route ID", r.getName());
        values.put("providerId", new Long(r.getProviderId()));
        values.put("categoryId", new Long(r.getCategoryId()));
        values.put("sources", r.getSources());
        values.put("destinations", r.getDestinations());
        values.put("trafficMode", new Integer(r.getTrafficMode()));
        values.put("billing", new Byte(r.getBilling()));
        values.put("isArchiving", new Boolean(r.isArchiving()));
        values.put("active", new Boolean(r.isActive()));
        values.put("suppressDeliveryReports", new Boolean(r.isSuppressDeliveryReports()));
        values.put("srcSmeId", r.getSrcSmeId());
        values.put("notes", r.getNotes());
        values.put("isTransit", new Boolean(r.isTransit()));
    }
}
