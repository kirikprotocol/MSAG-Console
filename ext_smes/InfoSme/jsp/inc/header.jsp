<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
	sme_menu_begin(out);
  sme_menu_button(out, "mbMenu",  "stat",      "Statistics","");
	sme_menu_button(out, "mbMenu",  "apply",     "Apply",     "");
  sme_menu_button(out, "mbMenu",  "statuses",  "Statuses",  "");
	sme_menu_button(out, "mbMenu",  "options",   "Options",   "Global InfoSme options");
	sme_menu_button(out, "mbMenu",  "drivers",   "Drivers",   "DB drivers");
	sme_menu_button(out, "mbMenu",  "providers", "Providers", "Providers");
	sme_menu_button(out, "mbMenu",  "tasks",     "Tasks",     "Tasks list");
	sme_menu_button(out, "mbMenu",  "shedules",  "Shedules",  "Shedules");
	sme_menu_space(out);
	sme_menu_end(out);
%>
