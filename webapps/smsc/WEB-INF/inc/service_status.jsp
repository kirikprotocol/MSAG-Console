<%@ page import="ru.novosoft.smsc.admin.service.ServiceInfo,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  ru.novosoft.smsc.admin.route.SmeStatus,
					  ru.novosoft.smsc.admin.AdminException"%><%@ include file = "/WEB-INF/inc/show_sme_status.jsp"%><%!
String smeStatus(String serviceId)
{
	SmeStatus status = null;
	try {
		status = appContext.getSmeManager().smeStatus(serviceId);
	}
	catch (AdminException e)
	{}
	String elem_id = "CONNECTION_STATUSERVICE_" + StringEncoderDecoder.encode(serviceId);
	return "<span id=\"" + elem_id + "\" datasrc=#tdcConnStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\" "+ showSmeStatus(status);
}
String serviceStatus(String serviceId)
{
	byte status = ServiceInfo.STATUS_UNKNOWN;
	try {
		status = appContext.getHostsManager().getServiceInfo(serviceId).getStatus();
	} catch (AdminException e)
	{}
	String elem_id = "RUNNING_STATUSERVICE_" + StringEncoderDecoder.encode(serviceId);
	String result = "<span id=\"" + elem_id + "\" datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\">";
	if (isServiceStatusColored)
	{
		switch (status)
		{
			case ServiceInfo.STATUS_RUNNING:
				result += "<img src=\"" + CPATH + "/img/ic_running.gif\" title='running'></span>";
				break;
			case ServiceInfo.STATUS_STOPPING:
				result += "<img src=\"" + CPATH + "/img/ic_stopping.gif\" title='stopping'>";
				break;
			case ServiceInfo.STATUS_STOPPED:
				result += "<img src=\"" + CPATH + "/img/ic_stopped.gif\" title='stopped'>";
				break;
			case ServiceInfo.STATUS_STARTING:
				result += "<img src=\"" + CPATH + "/img/ic_starting.gif\" title='starting'>";
				break;
			default:
				result += "<img src=\"" + CPATH + "/img/ic_unknown.gif\" title='unknown'>";
				break;
		}
	}
	else
	{
		switch (status)
		{
			case ServiceInfo.STATUS_RUNNING:
				result += "running";
				break;
			case ServiceInfo.STATUS_STOPPING:
				result += "stopping";
				break;
			case ServiceInfo.STATUS_STOPPED:
				result += "stopped";
				break;
			case ServiceInfo.STATUS_STARTING:
				result += "starting";
				break;
			default:
				result += "unknown";
				break;
		}
	}
	result += "</span>";
	return result;
}
%><OBJECT id="tdcStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="<%=CPATH%>/services/statuses.jsp?colored=<%=isServiceStatusColored%>">
	<PARAM NAME="UseHeader" VALUE="True">
	<PARAM NAME="TextQualifier" VALUE='"'>
</OBJECT><OBJECT id="tdcConnStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="<%=CPATH%>/services/connected_statuses.jsp">
	<PARAM NAME="UseHeader" VALUE="True">
	<PARAM NAME="TextQualifier" VALUE='"'>
</OBJECT><script>
function refreshStatus()
{
	document.all.tdcStatuses.DataURL = document.all.tdcStatuses.DataURL;
	document.all.tdcStatuses.reset();
	document.all.tdcConnStatuses.DataURL = document.all.tdcConnStatuses.DataURL;
	document.all.tdcConnStatuses.reset();
	window.setTimeout(refreshStatus, 5000);
}
refreshStatus();
</script>