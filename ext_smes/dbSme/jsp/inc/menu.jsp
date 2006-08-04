<%@ page import="ru.novosoft.smsc.admin.service.ServiceInfo"%><%@ include file="/WEB-INF/inc/sme_menu.jsp"%><%
	sme_menu_begin(out);
	sme_menu_button(out, "menuSelection",  "overview",	"Home",		  "Home");
	sme_menu_button(out, "menuSelection",  "params",	"Parameters", "Common parameters");
	sme_menu_button(out, "menuSelection",  "drivers",	"Drivers",	  "Datasource drivers");
	sme_menu_button(out, "menuSelection",  "providers",	"Providers",  "Data providers");
	sme_menu_space(out);
	if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0)
	{
		sme_menu_button(out, "mbStart",  "Start",  "Start",  "Start DbSme", "", !bean.isOnline());
		sme_menu_button(out, "mbStop",   "Stop",   "Stop",   "Stop DbSme",  "",  bean.isOnline());
	}
	sme_menu_end(out);
	if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0)
	{
    %><script type="text/javascript">
        function refreshDBSmeStartStopButtonsStatus()
        {
            var status = document.getElementById('RUNNING_STATUSERVICE_<%=ServiceIDForShowStatus%>').innerText;
            document.getElementById('mbStart').disabled = (status != "<%= getLocString("common.statuses.offine") %>");
            document.getElementById('mbStop').disabled  = (status == "<%= getLocString("common.statuses.offline") %>");
            window.setTimeout(refreshDBSmeStartStopButtonsStatus, 5000);
        }
        refreshDBSmeStartStopButtonsStatus();
    </script><%
	}
%>