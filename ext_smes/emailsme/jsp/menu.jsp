<%@ page import="ru.novosoft.smsc.admin.service.ServiceInfo"%>
<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
  sme_menu_begin(out);
  sme_menu_button(out, "mbMenu",  "apply",     "Home",      "");
  sme_menu_button(out, "mbMenu",  "options",   "Options",   "Global EMailSme options");
  sme_menu_button(out, "mbMenu",  "drivers",   "Drivers",   "Datasource drivers");
  sme_menu_button(out, "mbMenu",  "profiles",  "Profiles",  "User profiles");
  sme_menu_space(out);
  if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0)
  {
    sme_menu_button(out, "mbStart",  "Start",  "Start",  "Start DbSme", "return noValidationSubmit(this);", !bean.isServiceOnline());
    sme_menu_button(out, "mbStop",   "Stop",   "Stop",   "Stop DbSme",  "return noValidationSubmit(this);", bean.isServiceOnline());
  }
  sme_menu_end(out);
  if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0)
  {
		%><script>
			function refreshWSmeStartStopButtonsStatus()
			{
				document.getElementById('mbStart').disabled = (document.getElementById('RUNNING_STATUSERVICE_<%=ServiceIDForShowStatus%>').innerText != "stopped");
				document.getElementById('mbStop').disabled = (document.getElementById('RUNNING_STATUSERVICE_<%=ServiceIDForShowStatus%>').innerText != "running");
				window.setTimeout(refreshWSmeStartStopButtonsStatus, 500);
			}
			refreshWSmeStartStopButtonsStatus();
		</script><%
  }
%>
