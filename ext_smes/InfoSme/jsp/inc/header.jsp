<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
	sme_menu_begin(out);
    if (bean.isUserAdmin(request)) {
	    sme_menu_button(out, "mbMenu",  "apply",      getLocString("infosme.menu.statuses"), "");
      sme_menu_button(out, "mbMenu",  "options",    getLocString("infosme.menu.options"), "");
    }
    sme_menu_button(out, "mbMenu",  "tasks",      getLocString("infosme.menu.tasks"),      "");
    if (bean.isUserAdmin(request)) {
      sme_menu_button(out, "mbMenu",  "shedules",   getLocString("infosme.menu.schedules"),  "");
      sme_menu_button(out, "mbMenu",  "providers",  getLocString("infosme.menu.providers"),  "");
      sme_menu_button(out, "mbMenu",  "drivers",    getLocString("infosme.menu.drivers"),    "");
      sme_menu_button(out, "mbMenu",  "retryPolicies",    getLocString("infosme.menu.retry_policies"), "");
    }

    sme_menu_button(out, "mbMenu",  "deliveries", getLocString("infosme.menu.deliveries"), "");
    if (bean.isBlackListEnabled())
      sme_menu_button(out, "mbMenu",  "black_list", getLocString("infosme.menu.black_list"), "");

    sme_menu_space(out);
    sme_menu_button(out, "mbMenu",  "stat",      getLocString("infosme.menu.statistics"),"");
	sme_menu_end(out);
%>
