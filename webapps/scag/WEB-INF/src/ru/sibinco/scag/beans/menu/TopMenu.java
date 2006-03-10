/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.beans.menu;

import java.util.ArrayList;
import java.util.Collection;

/**
 * The <code>TopMenu</code> class represents
 * <p><p/>
 * Date: 04.07.2005
 * Time: 11:44:16
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class TopMenu  extends ArrayList {

    private static org.apache.log4j.Category category = org.apache.log4j.Category.getInstance(TopMenu.class);

    public TopMenu() {

        try {
            Collection scagItems = new ArrayList();
            scagItems.add(new MenuItem("menu.scag.status", "/gw/status/index.jsp"));
            scagItems.add(new MenuItem("menu.scag.config", "/gw/config/index.jsp"));
            scagItems.add(new MenuItem("menu.scag.categories", "/gw/categories/index.jsp"));
            scagItems.add(new MenuItem("menu.scag.logging", "/gw/logging/index.jsp"));
            scagItems.add(new MenuItem("menu.scag.users", "/gw/users/index.jsp"));
            add(new MenuItem("menu.scag", "", scagItems));

            Collection scagServices = new ArrayList();
            scagServices.add(new MenuItem("menu.services.providers", "/services/index.jsp"));
            scagServices.add(new MenuItem("menu.scag.operators", "/operators/index.jsp"));
            add(new MenuItem("menu.services", "", scagServices));

            Collection endpointItems = new ArrayList();
            endpointItems.add(new MenuItem("menu.endpoint.services","/endpoints/services/index.jsp"));
            endpointItems.add(new MenuItem("menu.endpoint.services.monitor","","/endpoints/services/monitor/index.jsp', 'svcmon', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=640');"));
            endpointItems.add(new MenuItem("menu.endpoint.centers","/endpoints/centers/index.jsp"));
            endpointItems.add(new MenuItem("menu.endpoint.centers.monitor","","/endpoints/centers/monitor/index.jsp', 'scmon', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=640');"));
            add(new MenuItem("menu.endpoint", "", endpointItems));

            Collection routingItems = new ArrayList();
            routingItems.add(new MenuItem("menu.routing.subjects", "/routing/subjects/index.jsp"));
            routingItems.add(new MenuItem("menu.routing.routes", "/routing/routes/index.jsp"));
            routingItems.add(new MenuItem("menu.routing.tracer", "/routing/tracer/index.jsp"));
            add(new MenuItem("menu.routing", "", routingItems));

            Collection statisticItems = new ArrayList();
            statisticItems.add(new MenuItem("menu.stat.monitor","", "/stat/monitor/index.jsp','perfmon', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=580');"));
            statisticItems.add(new MenuItem("menu.stat.stat","/stat/stat/index.jsp"));
            add(new MenuItem("menu.stat", "", statisticItems));

        } catch (Exception e) {
            category.error(e.getMessage());
        }
    }
}
