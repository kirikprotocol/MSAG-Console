<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
	sme_menu_begin(out);
    if (bean.isUserAdmin(request)) {
	    sme_menu_button(out, "mbMenu",  "apply",      "Statuses",   "");
      sme_menu_button(out, "mbMenu",  "options",    "Options",    "Global InfoSme options");
    }
    sme_menu_button(out, "mbMenu",  "tasks",      "Tasks",      "Tasks list");
    if (bean.isUserAdmin(request)) {
      sme_menu_button(out, "mbMenu",  "shedules",   "Schedules",  "Schedules");
      sme_menu_button(out, "mbMenu",  "providers",  "Providers",  "Providers");
      sme_menu_button(out, "mbMenu",  "drivers",    "Drivers",    "DB drivers");
      sme_menu_button(out, "mbMenu",  "retryPolicies",    "Retry Policies",    "Retry Policies");
    }

    sme_menu_button(out, "mbMenu",  "deliveries", "Deliveries", "Message deliveries");
    if (bean.isBlackListEnabled())
      sme_menu_button(out, "mbMenu",  "black_list", "Black List", "Black List");

    sme_menu_space(out);
    sme_menu_button(out, "mbMenu",  "messages",  "Messages",  "Messages set");
    sme_menu_button(out, "mbMenu",  "stat",      "Statistics","");
	sme_menu_end(out);
%>
