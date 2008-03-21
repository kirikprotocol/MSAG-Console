/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.novosoft.util.menu;

import ru.novosoft.smsc.admin.profiler.SupportExtProfile;

import java.util.ArrayList;
import java.util.Collection;


public class TopMenu extends ArrayList {

    private static org.apache.log4j.Category category = org.apache.log4j.Category.getInstance(TopMenu.class);

    public static String installationId = "";

    public static void setInstallationId(String installationId) {
        TopMenu.installationId = installationId;
    }

    public TopMenu() {

        try {
            Collection smsItems = new ArrayList();
            smsItems.add(new MenuItem("menu.smsc.configuration", "/smsc/smsc_service/index.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.reschedule", "/smsc/reshedule/index.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.directives", "/smsc/directives/index.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.status", "/smsc/smsc_service/status.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.logging", "/smsc/smsc_service/logging.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.maplimits", "/smsc/smsc_service/maplimits.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.users", "/smsc/users/index.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.resources", "/smsc/locale_resources/index.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.journal", "/smsc/journal/index.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.closedgroups", "/smsc/closed_groups/index.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.timezones", "/smsc/timezones/index.jsp", MenuItem.VISIBLE_IN_ALL));
            if (SupportExtProfile.enabled)
              smsItems.add(new MenuItem("menu.smsc.smscenters", "/smsc/smscenters/index.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.regions", "/smsc/regions/regions.jsp", MenuItem.VISIBLE_IN_ALL));
            smsItems.add(new MenuItem("menu.smsc.switch", "/smsc/smsc_service/switch.jsp", MenuItem.VISIBLE_IN_HA));
            smsItems.add(new MenuItem("menu.smsc.fraud", "/smsc/fraud/fraud.jsp", MenuItem.VISIBLE_IN_ALL));
            add(new MenuItem("menu.smsc", "", smsItems));

            Collection statisticItems = new ArrayList();
            statisticItems.add(new MenuItem("menu.stat.perfmon", "", "/smsc/perfmon/index.jsp','perfmon" + installationId + "', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=580');"));
            statisticItems.add(new MenuItem("menu.stat.topmon", "", "/smsc/topmon/index.jsp', 'topmon" + installationId + "', 'channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=640');"));
            statisticItems.add(new MenuItem("menu.stat.smsstat", "/smsc/smsstat/index.jsp", MenuItem.VISIBLE_IN_ALL));
            statisticItems.add(new MenuItem("menu.stat.statexport", "/smsc/smsstat/export.jsp", MenuItem.VISIBLE_IN_ALL));
            statisticItems.add(new MenuItem("menu.stat.statcsv", "/smsc/smsstat/index.jsp?csv=true", MenuItem.VISIBLE_IN_ALL));
            statisticItems.add(new MenuItem("menu.stat.daemon", "/smsc/smsview/daemon.jsp", MenuItem.VISIBLE_IN_ALL));
            statisticItems.add(new MenuItem("menu.stat.smsview", "/smsc/smsview/index.jsp", MenuItem.VISIBLE_IN_ALL));
            statisticItems.add(new MenuItem("menu.stat.smsexport", "/smsc/smsview/export.jsp", MenuItem.VISIBLE_IN_ALL));
            add(new MenuItem("menu.stat", "", statisticItems));

            Collection routingItems = new ArrayList();
            routingItems.add(new MenuItem("menu.routing.aliases", "/smsc/aliases/index.jsp", MenuItem.VISIBLE_IN_ALL));
            routingItems.add(new MenuItem("menu.routing.subjects", "/smsc/subjects/index.jsp", MenuItem.VISIBLE_IN_ALL));
            routingItems.add(new MenuItem("menu.routing.routes", "/smsc/routes/index.jsp", MenuItem.VISIBLE_IN_ALL));
            routingItems.add(new MenuItem("menu.routing.providers", "/smsc/providers/index.jsp", MenuItem.VISIBLE_IN_ALL));
            routingItems.add(new MenuItem("menu.routing.categories", "/smsc/categories/index.jsp", MenuItem.VISIBLE_IN_ALL));
            routingItems.add(new MenuItem("menu.routing.acl", "/smsc/acl/index.jsp", MenuItem.VISIBLE_IN_ALL));
            routingItems.add(new MenuItem("menu.routing.tracer", "/smsc/tracer/index.jsp", MenuItem.VISIBLE_IN_ALL));
            add(new MenuItem("menu.routing", "", routingItems));

            Collection profileItems = new ArrayList();
            profileItems.add(new MenuItem("menu.profiles.list", "/smsc/profiles/index.jsp", MenuItem.VISIBLE_IN_ALL));
            profileItems.add(new MenuItem("menu.profiles.groups", "/smsc/profiles/groups.jsp", MenuItem.VISIBLE_IN_ALL));
            profileItems.add(new MenuItem("menu.profiles.lookup", "/smsc/profiles/lookup.jsp", MenuItem.VISIBLE_IN_ALL));
            if (SupportExtProfile.enabled)
              profileItems.add(new MenuItem("menu.profiles.black.nicks", "/smsc/profiles/blackNicks.jsp", MenuItem.VISIBLE_IN_ALL));
            add(new MenuItem("menu.profiles", "", profileItems));

            Collection snmpItems = new ArrayList();
            snmpItems.add(new MenuItem("menu.snmp.config", "/smsc/snmp/config.jsp", MenuItem.VISIBLE_IN_ALL));
            snmpItems.add(new MenuItem("menu.snmp.alarmStat", "/smsc/snmp/alarmStat.jsp", MenuItem.VISIBLE_IN_ALL));
            add(new MenuItem("menu.snmp", "", snmpItems));

            Collection serviceItems = new ArrayList();
            serviceItems.add(new MenuItem("menu.services.hosts", "/smsc/hosts/index.jsp", MenuItem.VISIBLE_IN_ALL));
            serviceItems.add(new MenuItem("menu.services.services", "/smsc/services/index.jsp", MenuItem.VISIBLE_IN_SINGLE));
            serviceItems.add(new MenuItem("menu.services.services", "/smsc/services/resourceGroups.jsp", MenuItem.VISIBLE_IN_HA));
            add(new MenuItem("menu.services", "", serviceItems));

            Collection optionItems = new ArrayList();
            optionItems.add(new MenuItem("menu.options.mscs", "/smsc/mscman/index.jsp", MenuItem.VISIBLE_IN_ALL));
            optionItems.add(new MenuItem("menu.options.dl", "/smsc/dl/index.jsp", MenuItem.VISIBLE_IN_ALL));
            optionItems.add(new MenuItem("menu.options.principals", "/smsc/dl/principals.jsp", MenuItem.VISIBLE_IN_ALL));
            add(new MenuItem("menu.options", "", optionItems));

            Collection helpItems = new ArrayList();
            helpItems.add(new MenuItem("menu.help.adminguide", "/admin_guide.jsp", MenuItem.VISIBLE_IN_ALL));
            add(new MenuItem("menu.help", "", helpItems));
        } catch (Exception e) {
            category.error(e.getMessage());
        }
    }
}
