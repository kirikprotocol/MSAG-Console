<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
int total_hosts_count = 1;
if(request.getParameter("mbAdd")!=null)
{
	response.sendRedirect("serviceAdd.jsp");
	return;
} else if(request.getParameter("mbDelete")!=null)
{
}

%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
STATUS.append("hosts:").append(total_hosts_count);
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages);
%>
<input type=hidden ID=jbutton value="jbutton">
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
</script>
<h1>Services list</h1>
<table class=rep0 cellspacing=1 width="100%">
<col width="1%">
<col width="60%" align=left>
<col width="20%" align=left>
<col width="20%" align=center>
<thead>
<tr>
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


String hostName = "host "+serviceId;
String serviceControl = (row == 0) ? "start" : "stop";
String serviceStatus = (row == 0) ? "<span class='C800'>stopped</span>" : "<span class='C080'>runned</span>";
%>
<tr class=row<%=row&1%>>
	<td class=check><input class=check type=checkbox name=serviceIds></td>
	<td class=name><a href="#" title="View service info" onClick="return viewService('<%=serviceId%>');"><%=serviceId%></a></td>
	<td class=name><a href="#" title="View host info" onClick="return viewHost('<%=hostName%>');"><%=hostName%></a></td>
	<td><%=serviceStatus%></td>
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
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
