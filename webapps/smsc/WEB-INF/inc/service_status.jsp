<%@ page import="ru.novosoft.smsc.admin.service.ServiceInfo"%><%!
String serviceStatus(String serviceId, byte status, String elem_id)
{
	switch (status)
	{
		case ServiceInfo.STATUS_RUNNING:
			return "<span id=\"" + elem_id + "\" class=C080 datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\">running</span>";
		case ServiceInfo.STATUS_STOPPING:
			return "<span id=\"" + elem_id + "\" class=C008 datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\">stopping</span>";
		case ServiceInfo.STATUS_STOPPED:
			return "<span id=\"" + elem_id + "\" class=C800 datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\">stopped</span>";
		case ServiceInfo.STATUS_STARTING:
			return "<span id=\"" + elem_id + "\" class=C0F0 datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\">starting</span>";
		default:
			return "<span id=\"" + elem_id + "\" class=C000 datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(serviceId) + "\">unknown</span>";
	}
}
%><OBJECT id="tdcStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="<%=CPATH%>/services/statuses.jsp">
	<PARAM NAME="UseHeader" VALUE="True">
	<PARAM NAME="TextQualifier" VALUE='"'>
</OBJECT><script>
function refreshStatus()
{
	document.all.tdcStatuses.DataURL = document.all.tdcStatuses.DataURL;
	document.all.tdcStatuses.reset();
	window.setTimeout(refreshStatus, 5000);
}
refreshStatus();
</script>