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
            add(new MenuItem("menu.smsc", "", smsItems));

            Collection statisticItems = new ArrayList();
            statisticItems.add(new MenuItem("menu.stat.perfmon","", "/smsc/perfmon/index.jsp','perfmon', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=580');"));
            statisticItems.add(new MenuItem("menu.stat.topmon","", "/smsc/topmon/index.jsp', 'topmon', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=640');"));
            statisticItems.add(new MenuItem("menu.stat.smsstat", "/smsc/smsstat/index.jsp"));
            statisticItems.add(new MenuItem("menu.stat.statexport", "/smsc/smsstat/export.jsp"));
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
            add(new MenuItem("menu.services", "", serviceItems));

            Collection optionItems = new ArrayList();
            optionItems.add(new MenuItem("menu.options.mscs", "/smsc/mscman/index.jsp"));
            optionItems.add(new MenuItem("menu.options.dl", "/smsc/dl/index.jsp"));
            optionItems.add(new MenuItem("menu.options.principals", "/smsc/dl/principals.jsp"));
            add(new MenuItem("menu.options", "", optionItems));

            Collection helpItems = new ArrayList();
            helpItems.add(new MenuItem("menu.help.adminguide", "/admin_guide.jsp"));
            add(new MenuItem("menu.help", "", helpItems));

/*            smsItems.add(new MenuItem("top_menu.smsc_service", "/smsc/smsc_service/index.jsp"));
            smsItems.add(new MenuItem("top_menu.reshedule", "/smsc/reshedule/index.jsp"));
            smsItems.add(new MenuItem("top_menu.directives", "/smsc/directives/index.jsp"));
            smsItems.add(new MenuItem("top_menu.smsc_service.status", "/smsc/smsc_service/status.jsp"));
            smsItems.add(new MenuItem("top_menu.smsc_service.logging", "/smsc/smsc_service/logging.jsp"));
            smsItems.add(new MenuItem("top_menu.smsc_service.users", "/smsc/users/index.jsp"));
            smsItems.add(new MenuItem("top_menu.locale_resources", "/smsc/locale_resources/index.jsp"));
            smsItems.add(new MenuItem("top_menu.journal", "/smsc/journal/index.jsp"));
            add(new MenuItem("top_menu.smsc", "", smsItems));

            Collection statisticItems = new ArrayList();
            statisticItems.add(new MenuItem("top_menu.stat.perfmon","", "/smsc/perfmon/index.jsp','perfmon', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=580');"));
            statisticItems.add(new MenuItem("top_menu.stat.topmon","", "/smsc/topmon/index.jsp', 'topmon', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=640');"));
            statisticItems.add(new MenuItem("top_menu.stat.smsstat", "/smsc/smsstat/index.jsp"));
            statisticItems.add(new MenuItem("top_menu.stat.smsstat.export", "/smsc/smsstat/export.jsp"));
            statisticItems.add(new MenuItem("top_menu.stat.smsview.daemon", "/smsc/smsview/daemon.jsp"));
            statisticItems.add(new MenuItem("top_menu.stat.smsview", "/smsc/smsview/index.jsp"));
            statisticItems.add(new MenuItem("top_menu.stat.export", "/smsc/smsview/export.jsp"));
            add(new MenuItem("top_menu.statistic", "", statisticItems));

            Collection routingItems = new ArrayList();
            routingItems.add(new MenuItem("top_menu.routing.aliases", "/smsc/aliases/index.jsp"));
            routingItems.add(new MenuItem("top_menu.routing.subjects", "/smsc/subjects/index.jsp"));
            routingItems.add(new MenuItem("top_menu.routing.routes", "/smsc/routes/index.jsp"));
            routingItems.add(new MenuItem("top_menu.routing.providers", "/smsc/providers/index.jsp"));
            routingItems.add(new MenuItem("top_menu.routing.categories", "/smsc/categories/index.jsp"));
            routingItems.add(new MenuItem("top_menu.routing.acl", "/smsc/acl/index.jsp"));
            routingItems.add(new MenuItem("top_menu.routing.tracer", "/smsc/tracer/index.jsp"));
            add(new MenuItem("top_menu.routing", "", routingItems));

            Collection profileItems = new ArrayList();
            profileItems.add(new MenuItem("top_menu.profile.profiles", "/smsc/profiles/index.jsp"));
            profileItems.add(new MenuItem("top_menu.profile.groups", "/smsc/profiles/groups.jsp"));
            profileItems.add(new MenuItem("top_menu.profile.lookup", "/smsc/profiles/lookup.jsp"));
            add(new MenuItem("top_menu.profile", "", profileItems));

            Collection serviceItems = new ArrayList();
            serviceItems.add(new MenuItem("top_menu.service.hosts", "/smsc/hosts/index.jsp"));
            serviceItems.add(new MenuItem("top_menu.services", "/smsc/services/index.jsp"));
            add(new MenuItem("top_menu.service", "", serviceItems));

            Collection optionItems = new ArrayList();
            optionItems.add(new MenuItem("top_menu.option.mscman", "/smsc/mscman/index.jsp"));
            optionItems.add(new MenuItem("top_menu.option.dl", "/smsc/dl/index.jsp"));
            optionItems.add(new MenuItem("top_menu.option.dl.principals", "/smsc/dl/principals.jsp"));
            add(new MenuItem("top_menu.option", "", optionItems));

            Collection helpItems = new ArrayList();
            helpItems.add(new MenuItem("top_menu.help.admin_guide", "/admin_guide.jsp"));
            add(new MenuItem("top_menu.help", "", helpItems)); */

        } catch (Exception e) {
            category.error(e.getMessage());
        }
    }
}
