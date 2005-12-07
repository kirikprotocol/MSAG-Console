<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.Index,
                java.net.URLEncoder,
                ru.novosoft.smsc.jsp.SMSCJspException,
                ru.novosoft.smsc.jsp.SMSCErrors,
                ru.novosoft.smsc.jsp.PageBean"%>
<%
TITLE = getLocString("host.title");
switch (bean.process(request))
{
  case PageBean.RESULT_OK:
    break;
	case Index.RESULT_DONE:
		response.sendRedirect(CPATH+"/hosts/index.jsp");
		return;
	case Index.RESULT_VIEW:
		response.sendRedirect("hostView.jsp?hostName=" + URLEncoder.encode(bean.getHostName()));
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("hostAdd.jsp");
		return;
	case Index.RESULT_ERROR:
		// do nothing there, show errors below
		break;
	default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_HOSTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",  "host.add",  "host.addHint");
page_menu_button(session, out, "mbDelete", "host.delete", "host.deleteHint");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<input type=hidden name=hostName>

<script>
function viewHost(hostName)
{
	document.all.jbutton.value = "view";
	document.all.jbutton.name = "mbView";
	opForm.hostName.value = hostName;
	opForm.submit();
	return false;
}
</script>
<span class=CF00></span>
<table class=list cellspacing=0 cellpadding=0>
<col width="1%">
<col width="79%">
<col width="1%">
<col width="10%">
<thead>
<tr>
	<th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th><%=getLocString("common.sortmodes.name")%></th>
	<th><%=getLocString("common.sortmodes.port")%></th>
	<th title="<%=getLocString("common.sortmodes.services")+" ("+getLocString("common.sortmodes.total")+" / "+getLocString("common.sortmodes.running")+")"%>"><%=getLocString("common.sortmodes.services")%></th>
</tr>
</thead>
<tbody>
<%{
int row=0;
List hostIds = Arrays.asList(bean.getHostIds());
Collection hostNames = bean.getHostNames();
for(Iterator i = hostNames.iterator(); i.hasNext();row++)
{
String hostName = (String) i.next();
%>
<tr class=row<%=row&1%>>
	<td class=check><input class=check type=checkbox name=hostIds value="<%=hostName%>" <%=hostIds.contains(hostName) ? "checked" : ""%>></td>
	<td class=name><a href="javascript:viewHost('<%=hostName%>')" title="<%=getLocString("host.viewTitle")%>"><%=hostName%></a></td>
	<td class=num><%=bean.getHostPort(hostName)%></td>
	<td align=center><span class=C00F title="<%=getLocString("common.sortmodes.total")%>"><%=bean.getServicesTotal(hostName)%></span> / <span class=C080 title="<%=getLocString("common.sortmodes.running")%>"><%=bean.getServicesRunning(hostName)%></span></td>
</tr>
<%}}%>
</tbody>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",  "host.add",  "host.addHint");
page_menu_button(session, out, "mbDelete", "host.delete", "host.deleteHint");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>