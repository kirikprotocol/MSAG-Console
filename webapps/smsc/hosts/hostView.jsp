<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.HostView"/>
<jsp:setProperty name="bean" property="*"/>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.HostView, ru.novosoft.smsc.admin.service.ServiceInfo"%>
<%
switch (bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case HostView.RESULT_OK:
		STATUS.append("Ok");
		break;
	case HostView.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case HostView.RESULT_ERROR:
		// do nothing there, show errors below
		STATUS.append("Error");
		break;
	case HostView.RESULT_EDIT:
		response.sendRedirect("hostEdit.jsp?hostName="+bean.getHostName());
		return;
	case HostView.RESULT_VIEW:
		response.sendRedirect(CPATH+"/services/index.jsp?serviceId="+bean.getServiceId());
		return;
	case HostView.RESULT_ADD_SERVICE:
		response.sendRedirect(CPATH+"/services/addService.jsp?hostName="+bean.getHostName());
		return;
	default:
		STATUS.append("Error");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}

%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_HOSTS";

%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<input type=hidden name=hostName value="<%=bean.getHostName()%>">
<input type=hidden name=serviceId>
<input type=hidden ID=jbutton value="jbutton">
<script>
function refreshStatus()
{
	document.all.tdcStatuses.DataURL = document.all.tdcStatuses.DataURL;
	document.all.tdcStatuses.reset();
	window.setTimeout(refreshStatus, 5000);
}
function viewService(serviceId)
{
	document.all.jbutton.name = "mbView";
	opForm.serviceId.value = serviceId;
	opForm.submit();
	return false;
}
</script>
<h1>host <span class=C00F><%=bean.getHostName()%>:<%=bean.getPort()%></span> view</h1>
<span class=CF00><%@ include file="/WEB-INF/inc/messages.jsp"%></span>
<table class=frm1 cellspacing=0 width="100%">
<col width="15%" align=right>
<col width="85%">
<tr class=rowh><th colspan=2>information</th></tr>
<tr class=row0>
	<th>host name:</th>
	<td><%=bean.getHostName()%></td>
</tr>
<tr class=rowLast>
	<th>host port:</th>
	<td><%=bean.getPort()%></td>
</tr>
</table><%--div class=but0>
<input class=btn type=submit name=mbEdit value="Edit host" title="Edit host information">
<input class=btn type=submit name=mbCancel value="Cancel" title="Return to hosts list">
</div--%>
<h1>Services</h1>
<OBJECT id="tdcStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="servicesStatus.jsp?hostName=<%=bean.getHostName()%>">
	<PARAM NAME="UseHeader" VALUE="True">
	<PARAM NAME="TextQualifier" VALUE='"'>
</OBJECT>
<table class=rep0 cellspacing=1 width="100%">
<col width="1%">
<col width="60%" align=left>
<col width="20%" align=left>
<col width="20%" align=center>
<thead>
<tr>
	<th>&nbsp;</th>
	<th>service</th>
	<th>status</th>
</tr>
</thead>
<tbody>
<%{
int row=0;
List checkedServices = Arrays.asList(bean.getServiceIds());
for(Iterator i = bean.getServices().iterator(); i.hasNext(); row++)
{
ServiceInfo service = (ServiceInfo) i.next();
String serviceId = service.getId();
String serviceControl = (row == 0) ? "start" : "stop";
%>
<tr class=row<%=row&1%>>
	<td class=check><input class=check type=checkbox name=serviceIds value="<%=StringEncoderDecoder.encode(serviceId)%>" <%=checkedServices.contains(serviceId) ? "checked" : ""%>></td>
	<td class=name><a href="#" title="View service info" onClick="return viewService('<%=StringEncoderDecoder.encode(serviceId)%>');"><%=StringEncoderDecoder.encode(serviceId)%></a></td>
	<td><%
		switch (service.getStatus())
		{
			case ServiceInfo.STATUS_RUNNING:
				%><span class=C080 datasrc=#tdcStatuses DATAFORMATAS=html datafld="<%=StringEncoderDecoder.encode(serviceId)%>">running</span><%
				break;
			case ServiceInfo.STATUS_STOPPING:
				%><span class=C008 datasrc=#tdcStatuses DATAFORMATAS=html datafld="<%=StringEncoderDecoder.encode(serviceId)%>">stopping</span><%
				break;
			case ServiceInfo.STATUS_STOPPED:
				%><span class=C800 datasrc=#tdcStatuses DATAFORMATAS=html datafld="<%=StringEncoderDecoder.encode(serviceId)%>">stopped</span><%
				break;
			case ServiceInfo.STATUS_STARTING:
				%><span class=C0F0 datasrc=#tdcStatuses DATAFORMATAS=html datafld="<%=StringEncoderDecoder.encode(serviceId)%>">starting</span><%
				break;
			default:
				%><span class=C000 datasrc=#tdcStatuses DATAFORMATAS=html datafld="<%=StringEncoderDecoder.encode(serviceId)%>">unknown</span><%
				break;
		}
	%></td>
</tr>
<%}}%>
</tbody>
</table>
<div class=but0>
<input class=btn type=submit name=mbAddService value="Add service" title="Add service">
<input class=btn type=submit name=mbDelete value="Delete service(s)" title="Delete selected services">
&nbsp;&nbsp;&nbsp;&nbsp;
<input class=btn type=submit name=mbStartService value="Start service(s)" title="Start selected services">
<input class=btn type=submit name=mbStopService value="Stop service(s)" title="Stop selected services">
</div>
<script>
refreshStatus();
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
