<%@ page import="ru.novosoft.smsc.admin.service.ServiceInfo,
                 ru.novosoft.smsc.admin.Constants"%><%!
String wsmeStatus(byte status, String elem_id)
{
	switch (status)
	{
		case ServiceInfo.STATUS_RUNNING:
			return "<span id=\"" + elem_id + "\" class=C080 datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(Constants.WSME_SME_ID) + "\">running</span>";
		case ServiceInfo.STATUS_STOPPING:
			return "<span id=\"" + elem_id + "\" class=C008 datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(Constants.WSME_SME_ID) + "\">stopping</span>";
		case ServiceInfo.STATUS_STOPPED:
			return "<span id=\"" + elem_id + "\" class=C800 datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(Constants.WSME_SME_ID) + "\">stopped</span>";
		case ServiceInfo.STATUS_STARTING:
			return "<span id=\"" + elem_id + "\" class=C0F0 datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(Constants.WSME_SME_ID) + "\">starting</span>";
		default:
			return "<span id=\"" + elem_id + "\" class=C000 datasrc=#tdcStatuses DATAFORMATAS=html datafld=\"" + StringEncoderDecoder.encode(Constants.WSME_SME_ID) + "\">unknown</span>";
	}
}
%><OBJECT id="tdcStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="wsme_status.jsp">
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
WSme is <%= wsmeStatus(bean.getWSmeStatus(), "WSME_STATUS_ELEM_ID")%>
<div class=secButtons>
<input class=btn type=button id=btnStart name=btnStart value="Start" title="Start WSme" <%=bean.getWSmeStatus() != ServiceInfo.STATUS_STOPPED ? "disabled" : ""%> onClick="return noValidationSubmit(this)">
<input class=btn type=button id=btnStop name=btnStop value="Stop" title="Stop  WSme" <%=bean.getWSmeStatus() != ServiceInfo.STATUS_RUNNING ? "disabled" : ""%> onClick="return noValidationSubmit(this)">
<script>
function refreshWSmeStartStopButtonsStatus()
{
	document.all.btnStart.disabled = (document.all.WSME_STATUS_ELEM_ID.innerText != "stopped");
	document.all.btnStop.disabled = (document.all.WSME_STATUS_ELEM_ID.innerText != "running");
	window.setTimeout(refreshWSmeStartStopButtonsStatus, 500);
}
refreshWSmeStartStopButtonsStatus();
</script>
</div>
