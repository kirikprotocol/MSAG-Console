<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
  sme_menu_begin(out);
  sme_menu_button(out, "mbMenu",  "apply",     "Statuses",  "");
  sme_menu_button(out, "mbMenu",  "options",   "Options",   "Global MCISme options");
  sme_menu_button(out, "mbMenu",  "drivers",   "Drivers",   "DB drivers");
  sme_menu_space(out);
  sme_menu_button(out, "mbMenu",  "stat",      "Statistics","");
  sme_menu_end(out);
%>
