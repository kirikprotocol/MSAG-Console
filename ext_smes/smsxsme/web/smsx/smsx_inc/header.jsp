<%@ page import="java.util.Date" %>
<%@ page import="ru.sibinco.smsx.stats.backend.SmsxRequest" %>
<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
  sme_menu_begin(out);
  if(reports.contains(SmsxRequest.ReportType.SMSX_USERS)) {
    sme_menu_button(out, "mbMenu",  "smsx_users",      getLocString("smsx.results.users.title"), "");
  }
  if(reports.contains(SmsxRequest.ReportType.WEB_DAILY)) {
    sme_menu_button(out, "mbMenu",  "smsx_web_daily",      getLocString("smsx.results.webDaily.title"), "");
  }
  if(reports.contains(SmsxRequest.ReportType.WEB_REGIONS)) {
    sme_menu_button(out, "mbMenu",  "smsx_web_regions",      getLocString("smsx.results.webRegions.title"), "");
  }
  if(reports.contains(SmsxRequest.ReportType.TRAFFIC)) {
    sme_menu_button(out, "mbMenu",  "smsx_traffic",      getLocString("smsx.results.traffic.title"), "");
  }

  sme_menu_space(out);
  sme_menu_end(out);
%>
