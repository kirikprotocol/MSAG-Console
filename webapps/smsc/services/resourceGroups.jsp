<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.services.Index,
                 ru.novosoft.smsc.jsp.smsc.services.ResGroups,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.ResGroups"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("services.title");
isServiceStatusNeeded = true;
isServiceStatusColored = true;
switch(bean.process(request))
{
	case Index.RESULT_DONE:
		response.sendRedirect("resourceGroups.jsp");
		return;
	case Index.RESULT_OK:

		break;
	case Index.RESULT_ERROR:

		break;
	case Index.RESULT_VIEW:
		response.sendRedirect(CPATH+"/esme_"+URLEncoder.encode(bean.getServiceId())+"/index.jsp");
		return;
	case ResGroups.RESULT_VIEW_NODES:
		response.sendRedirect(CPATH+"/hosts/nodesView.jsp?servName="+bean.getServiceId());
		return;
	default:

		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbDisconnectServices",  "common.buttons.disconnect",  "services.disconnectHint", "return confirm('" + getLocString("services.disconnectConfirm") + "')", bean.isSmscAlive());
page_menu_button(session, out, "mbStartService",  "common.buttons.online",  "services.startHint");
page_menu_button(session, out, "mbStopService",  "common.buttons.offline",  "services.stopHint");
page_menu_end(out);
%><div class=content>
<input type=hidden name=serviceId>
<input type=hidden name=hostId>
<input type=hidden name=serviceType>
<script>
function viewNodes(servId)
{
	document.all.jbutton.value = "view host";
	document.all.jbutton.name = "mbViewNodes";
	opForm.serviceId.value = servId;
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
</script>

<table class=list cellspacing=1 width="100%">
<col width="1%" align=center>
<%if (request.isUserInRole("services")) {%><col width="1%"><%}%>
<col width="1%" align=left>
<col width="1%" align=center>
<col width="1%" align=center>
<col width="1%" align=center>
<thead>
<tr>
	<th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<%if (request.isUserInRole("services")) {%><th>&nbsp;</th><%}%>
	<th><%=getLocString("common.sortmodes.service")%></th>
	<th colspan="2"><%=getLocString("common.sortmodes.status")%></th>
    <th>online node</th>
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
	<%if (request.isUserInRole("services") && (bean.isService(serviceId))) {%>
     <td class=name>
      <a  href="#" title="<%=getLocString("services.editSubTitle")%>" onClick="return viewService('<%=encodedServiceId%>');"><%=getLocString("common.links.edit")%>
      </a>
      </td>
    <%}%>
	<td class=name><%=encodedServiceId%></td>
	<td class=name>
<%= bean.isServiceDisabled(serviceId)
 ?
  "<img src=\"/images/ic_disable.gif\" width=10 height=10 title='" + getLocString("common.hints.disabled") + "'>"
 :
  "<img src=\"/images/ic_enable.gif\" width=10 height=10 title='" + getLocString("common.hints.enabled") + "'>"%></td>
	<td class=name><%=smeStatus(bean.getAppContext(), serviceId)%></td>
	<td class=name><%
		if (bean.isService(serviceId))
		{
			%><%=rgStatus(bean.getAppContext(), serviceId)%><%
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
page_menu_button(session, out, "mbDisconnectServices",  "common.buttons.disconnect",  "services.disconnectHint", "return confirm('" + getLocString("services.disconnectConfirm") + "')", bean.isSmscAlive());
page_menu_button(session, out, "mbStartService",  "common.buttons.online",  "services.startHint");
page_menu_button(session, out, "mbStopService",  "common.buttons.offline",  "services.stopHint");
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
