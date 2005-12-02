<%@ page import="ru.novosoft.smsc.admin.service.ServiceInfo,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  ru.novosoft.smsc.admin.route.SmeStatus,
					  ru.novosoft.smsc.admin.AdminException,
                 ru.novosoft.smsc.jsp.SMSCAppContext"%><%@ include file = "/WEB-INF/inc/show_sme_status.jsp"%><%!
String smeStatus(SMSCAppContext appContext, String serviceId)
{
	SmeStatus status = null;
	try {
		status = appContext.getSmeManager().smeStatus(serviceId);
	}
	catch (Throwable e)
	{}
	String elem_id = "CONNECTION_STATUSERVICE_" + StringEncoderDecoder.encode(serviceId);
	return "<span id=\"" + elem_id + "\" datasrc=#tdcConnStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\" "+ showSmeStatus(status);
}
String serviceStatus(SMSCAppContext appContext, String serviceId)
{
  String elem_id = "RUNNING_STATUSERVICE_" + StringEncoderDecoder.encode(serviceId);
  return serviceStatus(appContext, serviceId, elem_id);
}

String rgStatus(SMSCAppContext appContext, String serviceId)
{
  String elem_id = "RUNNING_STATUSERVICE_" + StringEncoderDecoder.encode(serviceId);
  return rgStatus(appContext, serviceId, elem_id);
}

String rgStatus(SMSCAppContext appContext, String serviceId, String elem_id)
{
	byte status = ServiceInfo.STATUS_UNKNOWN;
//    String node = "";
	try {
		status = appContext.getHostsManager().getServiceStatus(serviceId);
	} catch (Throwable e)
	{}
	String result = "<span id=\"" + elem_id + "\" datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\">";
		switch (status)
		{
			case ServiceInfo.STATUS_OFFLINE:
				result += getLocString("common.statuses.offline");
				break;
			case ServiceInfo.STATUS_ONLINE1:
				result += getLocString("common.statuses.online1");
				break;
			case ServiceInfo.STATUS_ONLINE2:
				result += getLocString("common.statuses.online2");
				break;
			default:
				result += getLocString("common.statuses.unknown");
				break;
		}
	result += "</span>";
	return result;
}

String serviceStatus(SMSCAppContext appContext, String serviceId, String elem_id)
{
	byte status = ServiceInfo.STATUS_UNKNOWN;
	try {
		status = appContext.getHostsManager().getServiceInfo(serviceId).getStatus();
	} catch (Throwable e)
	{}
	String result = "<span id=\"" + elem_id + "\" datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\">";
	switch (status)
	{
		case ServiceInfo.STATUS_OFFLINE:
			result += getLocString("common.statuses.offline");
			break;
		case ServiceInfo.STATUS_ONLINE1:
			result += getLocString("common.statuses.online1");
			break;
		case ServiceInfo.STATUS_ONLINE2:
			result += getLocString("common.statuses.online2");
			break;
		default:
			result += getLocString("common.statuses.unknown");
			break;
	}
	result += "</span>";
	return result;
}

String smscServStatus(SMSCAppContext appContext, String serviceId)
{
  String elem_id = "RUNNING_STATUSERVICE_" + StringEncoderDecoder.encode(serviceId);
  return rgStatus(appContext, serviceId, elem_id);
}

String smscServStatus(SMSCAppContext appContext, String serviceId, String elem_id)
{
	byte status = ServiceInfo.STATUS_UNKNOWN;
	try {
		status = appContext.getHostsManager().getServiceStatus(serviceId);
	} catch (Throwable e)
	{}
	String result = "<span id=\"" + elem_id + "\" datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\">";
		switch (status)
		{
			case ServiceInfo.STATUS_OFFLINE:
				result += getLocString("common.statuses.deactivated");
				break;
			case ServiceInfo.STATUS_ONLINE1:
				result += getLocString("common.statuses.activated");
				break;
			case ServiceInfo.STATUS_ONLINE2:
				result += getLocString("common.statuses.activated");
				break;
			default:
				result += getLocString("common.statuses.unknown");
				break;
		}
	result += "</span>";
	return result;
}

%><OBJECT id="tdcStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="<%=CPATH%>/services/statuses.jsp">
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