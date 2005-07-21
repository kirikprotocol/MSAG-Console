<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
  sme_menu_begin(out);
  sme_menu_button(out, "mbMenu",  "apply",     "Statuses",   "General service status");
  sme_menu_button(out, "mbMenu",  "options",   "Options",    "Global MCISme options");
  sme_menu_button(out, "mbMenu",  "circuits",  "Circuits",   "Global MSC circuits options");
  sme_menu_button(out, "mbMenu",  "templates", "Templates",  "Message templates");
  sme_menu_button(out, "mbMenu",  "rules",     "Rules",      "Redirect rules");
  sme_menu_button(out, "mbMenu",  "offsets",   "TZ Offsets", "TimeZone offsets");
  sme_menu_button(out, "mbMenu",  "drivers",   "Drivers",    "DB drivers");
  sme_menu_space(out);
  sme_menu_button(out, "mbMenu",  "stat",      "Statistics", "Messages statistics");
  sme_menu_end(out);
%>
