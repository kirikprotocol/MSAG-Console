<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.HostStoppingServices"/>
<jsp:setProperty name="bean" property="*"/>
<%bean.setAppContext((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"));%>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.HostView, ru.novosoft.smsc.admin.service.ServiceInfo"%>
<%
switch (bean.process())
{
	case HostView.RESULT_OK:
		STATUS.append("Ok");
		break;
	case HostView.RESULT_DONE:
		response.sendRedirect("hostView.jsp?hostName="+bean.getHostName());
		return;
	case HostView.RESULT_ERROR:
		STATUS.append("Error:<span class=statusError>"+bean.getErrorMessage()+"</span>");
		break;
	default:
		STATUS.append("Error:<span class=statusError>unknown status</span>");
}

%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_HOSTS";

%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<input type=hidden name=hostName value="<%=bean.getHostName()%>">
<input type=hidden name=serviceId>
<input type=hidden name=waiting value="<%=bean.isWaiting()%>">

<script>
function viewService(serviceId)
{
	document.all.jbutton.name = "mbView";
	opForm.serviceId.value = serviceId;
	opForm.submit();
	return false;
}
</script>
 
<table class=list cellspacing=1 width="100%">
<col width="60%" align=left>
<col width="20%" align=left>
<col width="20%" align=center>
<thead>
<tr>
	<th>service</th>
	<th>status</th>
</tr>
</thead>
<tbody>
<%{
int row=0;
Set checkedServices = new HashSet(Arrays.asList(bean.getServiceIds()));
for(Iterator i = bean.getServices().iterator(); i.hasNext(); row++)
{
ServiceInfo service = (ServiceInfo) i.next();
String serviceId = service.getId();
String serviceControl = (row == 0) ? "start" : "stop";
String serviceStatus = service.isRunning() ? "<span class='C080'>runned</span>" : "<span class='C800'>stopped</span>";
%>
<tr class=row<%=row&1%>>
	<td class=name><a href="#" title="View service info" onClick="return viewService('<%=StringEncoderDecoder.encode(serviceId)%>');"><%=StringEncoderDecoder.encode(serviceId)%></a></td>
	<td><%=serviceStatus%></td>
</tr>
<%}}%>
</tbody>
</table>
<div class=secButtons>
<input class=btn type=submit name=mbAddService value="Add service" title="Add service">
<input class=btn type=submit name=mbDelete value="Delete service(s)" title="Delete selected services">
&nbsp;&nbsp;&nbsp;&nbsp;
<input class=btn type=submit name=mbStartService value="Start service(s)" title="Start selected services">
<input class=btn type=submit name=mbStopService value="Stop service(s)" title="Stop selected services">
</div>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
