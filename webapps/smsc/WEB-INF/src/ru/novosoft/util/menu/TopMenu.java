/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.novosoft.util.menu;

import java.util.ArrayList;
import java.util.Collection;


public class TopMenu extends ArrayList {

    private static org.apache.log4j.Category category = org.apache.log4j.Category.getInstance(TopMenu.class);

    public TopMenu() {

        try {
            Collection smsItems = new ArrayList();
            smsItems.add(new MenuItem("menu.smsc.configuration", "/smsc/smsc_service/index.jsp"));
            smsItems.add(new MenuItem("menu.smsc.reschedule", "/smsc/reshedule/index.jsp"));
            smsItems.add(new MenuItem("menu.smsc.directives", "/smsc/directives/index.jsp"));
            smsItems.add(new MenuItem("menu.smsc.status", "/smsc/smsc_service/status.jsp"));
            smsItems.add(new MenuItem("menu.smsc.logging", "/smsc/smsc_service/logging.jsp"));
            smsItems.add(new MenuItem("menu.smsc.users", "/smsc/users/index.jsp"));
            smsItems.add(new MenuItem("menu.smsc.resources", "/smsc/locale_resources/index.jsp"));
            smsItems.add(new MenuItem("menu.smsc.journal", "/smsc/journal/index.jsp"));
			smsItems.add(new MenuItem("menu.smsc.switch", "/smsc/smsc_service/switch.jsp"));
            add(new MenuItem("menu.smsc", "", smsItems));

            Collection statisticItems = new ArrayList();
            statisticItems.add(new MenuItem("menu.stat.perfmon","", "/smsc/perfmon/index.jsp','perfmon', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=580');"));
            statisticItems.add(new MenuItem("menu.stat.topmon","", "/smsc/topmon/index.jsp', 'topmon', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=640');"));
            statisticItems.add(new MenuItem("menu.stat.smsstat", "/smsc/smsstat/index.jsp"));
            statisticItems.add(new MenuItem("menu.stat.statexport", "/smsc/smsstat/export.jsp"));
            statisticItems.add(new MenuItem("menu.stat.statcsv", "/smsc/smsstat/index.jsp?csv=true"));
            statisticItems.add(new MenuItem("menu.stat.daemon", "/smsc/smsview/daemon.jsp"));
            statisticItems.add(new MenuItem("menu.stat.smsview", "/smsc/smsview/index.jsp"));
            statisticItems.add(new MenuItem("menu.stat.smsexport", "/smsc/smsview/export.jsp"));
            add(new MenuItem("menu.stat", "", statisticItems));

            Collection routingItems = new ArrayList();
            routingItems.add(new MenuItem("menu.routing.aliases", "/smsc/aliases/index.jsp"));
            routingItems.add(new MenuItem("menu.routing.subjects", "/smsc/subjects/index.jsp"));
            routingItems.add(new MenuItem("menu.routing.routes", "/smsc/routes/index.jsp"));
            routingItems.add(new MenuItem("menu.routing.providers", "/smsc/providers/index.jsp"));
            routingItems.add(new MenuItem("menu.routing.categories", "/smsc/categories/index.jsp"));
            routingItems.add(new MenuItem("menu.routing.acl", "/smsc/acl/index.jsp"));
            routingItems.add(new MenuItem("menu.routing.tracer", "/smsc/tracer/index.jsp"));
            add(new MenuItem("menu.routing", "", routingItems));

            Collection profileItems = new ArrayList();
            profileItems.add(new MenuItem("menu.profiles.list", "/smsc/profiles/index.jsp"));
            profileItems.add(new MenuItem("menu.profiles.groups", "/smsc/profiles/groups.jsp"));
            profileItems.add(new MenuItem("menu.profiles.lookup", "/smsc/profiles/lookup.jsp"));
            add(new MenuItem("menu.profiles", "", profileItems));

            Collection serviceItems = new ArrayList();
            serviceItems.add(new MenuItem("menu.services.hosts", "/smsc/hosts/index.jsp"));
            serviceItems.add(new MenuItem("menu.services.services", "/smsc/services/index.jsp"));
			serviceItems.add(new MenuItem("menu.services.services", "/smsc/services/resourceGroups.jsp"));
            add(new MenuItem("menu.services", "", serviceItems));

            Collection optionItems = new ArrayList();
            optionItems.add(new MenuItem("menu.options.mscs", "/smsc/mscman/index.jsp"));
            optionItems.add(new MenuItem("menu.options.dl", "/smsc/dl/index.jsp"));
            optionItems.add(new MenuItem("menu.options.principals", "/smsc/dl/principals.jsp"));
            add(new MenuItem("menu.options", "", optionItems));

            Collection helpItems = new ArrayList();
            helpItems.add(new MenuItem("menu.help.adminguide", "/admin_guide.jsp"));
            add(new MenuItem("menu.help", "", helpItems));
        } catch (Exception e) {
            category.error(e.getMessage());
        }
    }
}
