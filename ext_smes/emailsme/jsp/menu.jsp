<%@ page import="ru.novosoft.smsc.admin.service.ServiceInfo"%>
<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
  sme_menu_begin(out);
  sme_menu_button(out, "mbMenu",  "apply",     "Home",      "");
  sme_menu_button(out, "mbMenu",  "options",   "Options",   "Global EMailSme options");
  sme_menu_button(out, "mbMenu",  "drivers",   "Drivers",   "Doatasource drivers");
  sme_menu_button(out, "mbMenu",  "profiles",  "Profiles",  "User profiles");
  sme_menu_space(out);
  if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0)
  {
    sme_menu_button(out, "mbStart",  "Start",  "Start",  "Start DbSme", "return noValidationSubmit(this);", bean.getServiceStatus() == ServiceInfo.STATUS_STOPPED);
    sme_menu_button(out, "mbStop",   "Stop",   "Stop",   "Stop DbSme",  "return noValidationSubmit(this);", bean.getServiceStatus() == ServiceInfo.STATUS_RUNNING);
  }
  sme_menu_end(out);
  if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0)
  {
		%><script>
			function refreshWSmeStartStopButtonsStatus()
			{
				document.all.mbStart.disabled = (document.all.RUNNING_STATUSERVICE_<%=ServiceIDForShowStatus%>.innerText != "stopped");
				document.all.mbStop.disabled = (document.all.RUNNING_STATUSERVICE_<%=ServiceIDForShowStatus%>.innerText != "running");
				window.setTimeout(refreshWSmeStartStopButtonsStatus, 500);
			}
			refreshWSmeStartStopButtonsStatus();
		</script><%
  }
%>
