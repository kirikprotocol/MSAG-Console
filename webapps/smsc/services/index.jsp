<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.services.Index,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Services";
isServiceStatusNeeded = true;
isServiceStatusColored = true;
switch(bean.process(request))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:

		break;
	case Index.RESULT_ERROR:

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

		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAddService",  "Add",  "Add service");
page_menu_button(session, out, "mbDelete", "Delete", "Delete selected services", "return confirm('Are you sure to delete all selected services?')");
page_menu_space(out);
page_menu_button(session, out, "mbDisconnectServices",  "Disconnect",  "Disconnect all selected services", "return confirm('Are you sure to disconnect all selected services?')", bean.isSmscAlive());
page_menu_button(session, out, "mbStartService",  "Start",  "Start selected services");
page_menu_button(session, out, "mbStopService",  "Stop",  "Stop selected services");
page_menu_end(out);
%><div class=content>
<input type=hidden name=serviceId>
<input type=hidden name=hostId>
<input type=hidden name=serviceType>
<script>
function viewHost(hostId)
{
	document.all.jbutton.value = "view host";
	document.all.jbutton.name = "mbViewHost";
	opForm.hostId.value = hostId;
	opForm.submit();
	return false;
}
function viewService(serviceId)
{
	document.all.jbutton.value = "view";
	document.all.jbutton.name = "mbView";
	opForm.serviceId.value = serviceId;
	opForm.submit();
	return false;
}
function editService(serviceId)
{
	document.all.jbutton.value = "edit";
	document.all.jbutton.name = "mbEdit";
	opForm.serviceId.value = serviceId;
	opForm.submit();
	return false;
}
</script>

<table class=list cellspacing=1 width="100%">
<col width="1%" align=center>
<%if (request.isUserInRole("services")) {%><col width="1%"><%}%>
<col align=left>
<%if (request.isUserInRole("services")) {%><col width="1%" align=left><%}%>
<col width="1%" align=center>
<col width="1%" align=center>
<col width="1%" align=center>
<thead>
<tr>
	<th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<%if (request.isUserInRole("services")) {%><th>&nbsp;</th><%}%>
	<th>service</th>
	<%if (request.isUserInRole("services")) {%><th>host</th><%}%>
	<th colspan="3">status</th>
</tr>
</thead>
<tbody>
<%{
int row = 0;
for(Iterator i = bean.getSmeIds().iterator(); i.hasNext(); row++)
{
String serviceId = (String) i.next();
String encodedServiceId = StringEncoderDecoder.encode(serviceId);

List serviceIds = Arrays.asList(bean.getServiceIds());
%>
<tr class=row<%=row&1%>>
	<td class=check><input class=check type=checkbox name=serviceIds value="<%=encodedServiceId%>" <%=serviceIds.contains(serviceId) ? "checked" : ""%>></td>
	<%if (request.isUserInRole("services")) {%><td class=name><a  href="#" title="Edit service parameters" onClick="return editService('<%=encodedServiceId%>');">edit</a></td><%}%>
	<td class=name><%
		if (bean.isServiceAdministrable(serviceId) && request.isUserInRole("services"))
		{
			%><a href="#" title="View service info" onClick="return viewService('<%=encodedServiceId%>');"><%=encodedServiceId%></a><%
		}
			else
		{
			%><%=encodedServiceId%><%
		}
	%></td>
	<%if (request.isUserInRole("services")) {
    %><td class=name><%
		if (bean.isService(serviceId))
		{
			%><a href="#" title="View host info" onClick="return viewHost('<%=bean.getHost(serviceId)%>');"><%=bean.getHost(serviceId)%></a><%
		} else
		{
			%>&nbsp;<%
		}
  	%></td>
  <%}%>
	<td class=name><%=bean.isServiceDisabled(serviceId) ? "<img src=\"/images/ic_disable.gif\" width=10 height=10 title='disabled'>" : "<img src=\"/images/ic_enable.gif\" width=10 height=10 title='enabled'>"%></td>
	<td class=name><%=smeStatus(bean.getAppContext(), serviceId)%></td>
	<td class=name><%
		if (bean.isService(serviceId))
		{
			%><%=serviceStatus(bean.getAppContext(), serviceId)%><%
		} else
		{
			%>&nbsp;<%
		}
	%></td>
</tr>
<%}}%>
</tbody>
</table>
</div>
<%
page_menu_begin(out);
if (request.isUserInRole("services")) {
  page_menu_button(session, out, "mbAddService",  "Add",  "Add service");
  page_menu_button(session, out, "mbDelete", "Delete", "Delete selected services", "return confirm('Are you sure to delete all selected services?');");
}
page_menu_space(out);
page_menu_button(session, out, "mbDisconnectServices",  "Disconnect",  "Disconnect all selected services", "return confirm('Are you sure to disconnect all selected services?')", bean.isSmscAlive());
page_menu_button(session, out, "mbStartService",  "Start",  "Start selected services");
page_menu_button(session, out, "mbStopService",  "Stop",  "Stop selected services");
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
