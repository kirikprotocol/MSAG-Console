<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="/WEB-INF/inc/service_status.jsp"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.HostView"/>
<jsp:setProperty name="bean" property="*"/>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.HostView, ru.novosoft.smsc.admin.service.ServiceInfo"%>
<%
switch (bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case HostView.RESULT_OK:
		STATUS.append("Services: " + bean.getServicesRunning() + " of " + bean.getServicesTotal() + " is running");
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
		response.sendRedirect(CPATH+"/esme_"+URLEncoder.encode(bean.getServiceId())+"/index.jsp");
		return;
	case HostView.RESULT_ADD_SERVICE:
		response.sendRedirect(CPATH+"/services/serviceAdd.jsp?hostName="+bean.getHostName());
		return;
	default:
		STATUS.append("Error");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}

TITLE = "host \""+bean.getHostName()+":"+bean.getPort()+"\" view";
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_HOSTS";

%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<input type=hidden name=hostName value="<%=bean.getHostName()%>">
<input type=hidden name=serviceId>

<script>
function viewService(serviceId)
{
	document.all.jbutton.name = "mbView";
	opForm.serviceId.value = serviceId;
	opForm.submit();
	return false;
}
</script>

<div class=secInfo>Host infromation</div>
<table class=secRep cellspacing=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<tbody>
<tr class=row0>
	<th>host name:</th>
	<td><%=bean.getHostName()%></td>
</tr>
<tr class=row1>
	<th>host port:</th>
	<td><%=bean.getPort()%></td>
</tr>
</tbody></table>
<%--div class=secButtons>
<input class=btn type=submit name=mbEdit value="Edit host" title="Edit host information">
<input class=btn type=submit name=mbCancel value="Cancel" title="Return to hosts list">
</div--%>
<div class=secList>Host Services</div>
<table class=secRep cellspacing=1 width="100%">
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
	<td><%=serviceStatus(serviceId, service.getStatus())%></td>
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
