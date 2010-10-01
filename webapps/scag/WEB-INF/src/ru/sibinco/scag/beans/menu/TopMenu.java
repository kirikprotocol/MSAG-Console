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
            scagItems.add(new MenuItem("menu.scag.status", "/gw/status/index.jsp",new String[]{"gw"}));
            scagItems.add(new MenuItem("menu.scag.config", "/gw/config/index.jsp",new String[]{"gw"}));
            scagItems.add(new MenuItem("menu.scag.logging", "/gw/logging/logging.jsp",new String[]{"gw"}));
            scagItems.add(new MenuItem("menu.scag.users", "/gw/users/index.jsp",new String[]{"super_admin"}));
            scagItems.add(new MenuItem("menu.stat.counters", "/stat/counters/index.jsp",new String[]{"gw"}));
            scagItems.add(new MenuItem("menu.stat.counters.ca", "/stat/counters/ca/index.jsp",new String[]{"gw"}));
            add(new MenuItem("menu.scag", "", scagItems, new String[]{"gw","super_admin"}));

            Collection scagServices = new ArrayList();
            scagServices.add(new MenuItem("menu.services.providers", "/services/index.jsp",new String[]{"management"}));
            scagServices.add(new MenuItem("menu.scag.operators", "/operators/index.jsp",new String[]{"management"}));
            add(new MenuItem("menu.services", "", scagServices,new String[]{"management"}));

            Collection endpointItems = new ArrayList();
            endpointItems.add(new MenuItem("menu.endpoint.services","/endpoints/services/index.jsp", new String[]{"routing"}));
            endpointItems.add(new MenuItem("menu.endpoint.metaservices","/endpoints/metaservices/index.jsp", new String[]{"routing"}));
            endpointItems.add(new MenuItem("menu.endpoint.services.monitor","","/endpoints/services/monitor/index.jsp', '', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=640');",new String[]{"stat"}));
            endpointItems.add(new MenuItem("menu.endpoint.centers","/endpoints/centers/index.jsp",new String[]{"routing"}));
            endpointItems.add(new MenuItem("menu.endpoint.metacenters","/endpoints/metacenters/index.jsp",new String[]{"routing"}));            
            endpointItems.add(new MenuItem("menu.endpoint.centers.monitor","","/endpoints/centers/monitor/index.jsp', '', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=640');",new String[]{"stat"}));
            add(new MenuItem("menu.endpoint", "", endpointItems, new String[]{"routing","stat"}));

            Collection routingItems = new ArrayList();
            Collection subjectItems = new ArrayList();
            subjectItems.add(new MenuItem("menu.routing.routes.SMPP", "/routing/subjects/index.jsp?transportId=1",new String[]{"routing"}));
            subjectItems.add(new MenuItem("menu.routing.routes.HTTP", "/routing/subjects/index.jsp?transportId=2",new String[]{"routing"}));
            /* Hide MMS
            subjectItems.add(new MenuItem("menu.routing.routes.MMS", "/routing/subjects/index.jsp?transportId=3",new String[]{"routing"}));
            */
            routingItems.add(new MenuItem("menu.routing.subjects", "", subjectItems,new String[]{"routing"}));
            routingItems.add(new MenuItem("menu.routing.options", "/routing/options/index.jsp",new String[]{"routing"}));
            Collection routesStatusItems = new ArrayList();
            routesStatusItems.add(new MenuItem("menu.routing.routes.SMPP", "/routing/routes/index.jsp?transportId=1",new String[]{"routing"}));
            routesStatusItems.add(new MenuItem("menu.routing.routes.HTTP", "/routing/routes/index.jsp?transportId=2",new String[]{"routing"}));
            /* Hide MMS 
            routesStatusItems.add(new MenuItem("menu.routing.routes.MMS", "/routing/routes/index.jsp?transportId=3",new String[]{"routing"}));
            */
            routingItems.add(new MenuItem("menu.routing.routes", "", routesStatusItems,new String[]{"routing"}));
            routingItems.add(new MenuItem("menu.routing.tracer", "/routing/tracer/index.jsp",new String[]{"routing"}));
            add(new MenuItem("menu.routing", "", routingItems,new String[]{"routing"}));

            Collection statisticItems = new ArrayList();
            statisticItems.add(new MenuItem("menu.stat.monitor","", "/stat/monitor/index.jsp', '', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=500,width=580');",new String[]{"stat"}));
            statisticItems.add(new MenuItem("menu.stat.stat","/stat/stat/index.jsp",new String[]{"stat"}));
            add(new MenuItem("menu.stat", "", statisticItems,new String[]{"stat"}));

        } catch (Exception e) {
            category.error(e.getMessage());
        }
    }
}
