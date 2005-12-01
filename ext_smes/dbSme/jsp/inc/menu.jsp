<%@ page import="ru.novosoft.smsc.admin.service.ServiceInfo"%><%@ include file="/WEB-INF/inc/sme_menu.jsp"%><%
	sme_menu_begin(out);
	sme_menu_button(out, "menuSelection",  "overview",	"Home",		  "Home",				"return noValidationSubmit(this);");
	sme_menu_button(out, "menuSelection",  "params",	"Parameters", "Common parameters",	"return noValidationSubmit(this);");
	sme_menu_button(out, "menuSelection",  "drivers",	"Drivers",	  "Datasource drivers", "return noValidationSubmit(this);");
	sme_menu_button(out, "menuSelection",  "providers",	"Providers",  "Data providers",		"return noValidationSubmit(this);");
	sme_menu_space(out);
	if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0)
	{
		sme_menu_button(out, "mbStart",  "Start",  "Start",  "Start DbSme", "return noValidationSubmit(this);", !bean.isOnline());
		sme_menu_button(out, "mbStop",   "Stop",   "Stop",   "Stop DbSme",  "return noValidationSubmit(this);",  bean.isOnline());
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
