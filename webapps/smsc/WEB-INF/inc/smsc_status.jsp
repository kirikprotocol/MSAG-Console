<%@ page import="ru.novosoft.smsc.admin.service.ServiceInfo,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  ru.novosoft.smsc.admin.Constants"%><%!
String smscStatus(byte status, String elem_id)
{
	switch (status)
	{
		case ServiceInfo.STATUS_RUNNING:
			return "<span id=\"" + elem_id + "\" datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(Constants.SMSC_SME_ID) + "\">running</span>";
		case ServiceInfo.STATUS_STOPPING:
			return "<span id=\"" + elem_id + "\" datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(Constants.SMSC_SME_ID) + "\">stopping</span>";
		case ServiceInfo.STATUS_STOPPED:
			return "<span id=\"" + elem_id + "\" datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(Constants.SMSC_SME_ID) + "\">stopped</span>";
		case ServiceInfo.STATUS_STARTING:
			return "<span id=\"" + elem_id + "\" datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(Constants.SMSC_SME_ID) + "\">starting</span>";
		default:
			return "<span id=\"" + elem_id + "\" datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(Constants.SMSC_SME_ID) + "\">unknown</span>";
	}
}
%><OBJECT id="tdcStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="<%=CPATH%>/smsc_service/smsc_status.jsp">
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