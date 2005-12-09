<%@ page import="ru.novosoft.smsc.admin.Constants"%><%
page_menu_begin(out);
page_menu_space(out);
page_menu_button(out, "btnStart",  "Start",  "Start WSme", "return noValidationSubmit(this);", !bean.isWSmeOnline());
page_menu_button(out, "btnStop",   "Stop",   "Stop WSme",  "return noValidationSubmit(this);", bean.isWSmeOnline());
page_menu_end(out);
%>
<script>
function refreshWSmeStartStopButtonsStatus()
{
	document.getElementById('btnStart').disabled = (document.getElementById('RUNNING_STATUSERVICE_<%=bean.getSmeId()%>').innerText != "stopped");
	document.getElementById('btnStop').disabled = (document.getElementById('RUNNING_STATUSERVICE_<%=bean.getSmeId()%>').innerText != "running");
	window.setTimeout(refreshWSmeStartStopButtonsStatus, 500);
}
refreshWSmeStartStopButtonsStatus();
</script>