<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="/WEB-INF/inc/service_status.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.services.Index"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Services List";
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		STATUS.append("Ok");
		break;
	case Index.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
	case Index.RESULT_VIEW:
		response.sendRedirect(CPATH+"/esme_"+URLEncoder.encode(bean.getServiceId())+"/index.jsp");
		return;
	case Index.RESULT_VIEW_HOST:
		response.sendRedirect(CPATH+"/hosts/hostView.jsp?hostName="+bean.getHostId());
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("serviceAdd.jsp" + (bean.getHostId() != null ? ("?hostName=" + bean.getHostId()) : ""));
		return;
	case Index.RESULT_EDIT:
		response.sendRedirect(CPATH+"/services/serviceEditSme.jsp?serviceId="+URLEncoder.encode(bean.getServiceId()));
		return;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<input type=hidden name=serviceId>
<input type=hidden name=hostId>
<input type=hidden name=serviceType>
<script>
function viewHost(hostId)
{
	document.all.jbutton.name = "mbViewHost";
	opForm.hostId.value = hostId;
	opForm.submit();
	return false;
}
function viewService(serviceId)
{
	document.all.jbutton.name = "mbView";
	opForm.serviceId.value = serviceId;
	opForm.submit();
	return false;
}
function editService(serviceId)
{
	document.all.jbutton.name = "mbEdit";
	opForm.serviceId.value = serviceId;
	opForm.submit();
	return false;
}
</script>

<table class=secRep cellspacing=1 width="100%">
<col width="1%">
<col width="1%">
<col width="60%" align=left>
<col width="20%" align=left>
<col width="20%" align=center>
<thead>
<tr>
	<th class=ico><img src="<%=CPATH%>/img/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th>&nbsp;</th>
	<th>service</th>
	<th>host</th>
	<th>status</th>
</tr>
</thead>
<tbody>
<%{
int row = 0;
for(Iterator i = bean.getSmeIds().iterator(); i.hasNext(); row++)
{
String serviceId = (String) i.next();
String encodedServiceId = StringEncoderDecoder.encode(serviceId);

String serviceControl = (row == 0) ? "start" : "stop";
String serviceStatus = (row == 0) ? "<span class='C800'>stopped</span>" : "<span class='C080'>runned</span>";
List serviceIds = Arrays.asList(bean.getServiceIds());
%>
<tr class=row<%=row&1%>>
	<td class=check><input class=check type=checkbox name=serviceIds value="<%=encodedServiceId%>" <%=serviceIds.contains(serviceId) ? "checked" : ""%>></td>
	<td><a  href="#" title="Edit service parameters" onClick="return editService('<%=encodedServiceId%>');">edit</a></td>
	<%
	if (bean.isServiceAdministrable(serviceId))
	{
		%><td class=name><a href="#" title="View service info" onClick="return viewService('<%=encodedServiceId%>');"><%=encodedServiceId%></a></td><%
		if (bean.isService(serviceId))
		{
			%><td class=name><a href="#" title="View host info" onClick="return viewHost('<%=bean.getHost(serviceId)%>');"><%=bean.getHost(serviceId)%></a></td><%
		} else
		{
			%><td class=name>&nbsp;</td><%
		}
	}
		else
	{
		%><td class=name><%=encodedServiceId%></td>
		<td class=name>&nbsp;</td><%
	}%>
	<td><%=serviceStatus(serviceId, bean.getServiceStatus(serviceId))%></td>
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
