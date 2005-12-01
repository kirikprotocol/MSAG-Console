<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.HostView"/>
<jsp:setProperty name="bean" property="*"/>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.HostView, ru.novosoft.smsc.admin.service.ServiceInfo,
                ru.novosoft.smsc.util.StringEncoderDecoder,
                java.util.*,
                java.net.URLEncoder,
                ru.novosoft.smsc.jsp.SMSCJspException,
                ru.novosoft.smsc.jsp.SMSCErrors,
                ru.novosoft.smsc.jsp.PageBean"%>
<%
isServiceStatusNeeded = true;
switch (bean.process(request))
{
  case PageBean.RESULT_OK:
    break;
	case HostView.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case HostView.RESULT_ERROR:
		// do nothing there, show errors below
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
	case HostView.RESULT_EDIT_SERVICE:
		response.sendRedirect(CPATH+"/services/serviceEditSme.jsp?serviceId="+URLEncoder.encode(bean.getServiceId()));
		return;
	default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
TITLE = getLocString("host.viewTitlePre") + " \"" + bean.getHostName() + ":" + bean.getPort() + "\"";
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_HOSTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAddService",  "services.add",  "services.addHint");
page_menu_button(session, out, "mbDelete", "services.delete", "services.deleteHint");
page_menu_space(out);
page_menu_button(session, out, "mbStartService",  "services.start",  "services.startHint");
page_menu_button(session, out, "mbStopService", "services.stop", "services.stopHint");
page_menu_end(out);
%>
<div class=content>
<input type=hidden name=hostName value="<%=bean.getHostName()%>">
<input type=hidden name=serviceId>

<script>
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
	document.all.jbutton.value = "edit service";
	document.all.jbutton.name = "mbEditService";
	opForm.serviceId.value = serviceId;
	opForm.submit();
	return false;
}
</script>

<div class=page_subtitle><%=getLocString("host.viewInfoSubTitle")%></div>
<table class=properties_list cellspacing=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<tbody>
<tr class=row0>
	<th><%=getLocString("host.name")%>:</th>
	<td><%=bean.getHostName()%></td>
</tr>
<tr class=row1>
	<th><%=getLocString("host.port")%>:</th>
	<td><%=bean.getPort()%></td>
</tr>
</tbody></table>
<%--div class=secButtons>
<input class=btn type=submit name=mbEdit value="Edit host" title="Edit host information">
<input class=btn type=submit name=mbCancel value="Cancel" title="Return to hosts list">
</div--%>
<div class=page_subtitle><%=getLocString("host.viewServSubTitle")%></div>
<table class=list cellspacing=1 width="100%">
<col width="1%">
<col width="1%">
<col width="70%" align=left>
<col width="28%" align=left>
<thead>
<tr>
	<th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th>&nbsp;</th>
	<th><%=getLocString("dl.service")%></th>
	<th><%=getLocString("dl.status")%></th>
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
String encodedServiceId = StringEncoderDecoder.encode(serviceId);
String serviceControl = (row == 0) ? "start" : "stop";
%>
<tr class=row<%=row&1%>>
	<td><input class=check type=checkbox name=serviceIds value="<%=encodedServiceId%>" <%=checkedServices.contains(serviceId) ? "checked" : ""%>></td>
	<td><a  href="#" title="<%=getLocString("host.editServHint")%>" onClick="return editService('<%=encodedServiceId%>');"><%=getLocString("common.buttons.edit")%></a></td>
	<td><a href="#" title="<%=getLocString("host.viewServInfo")%>" onClick="return viewService('<%=encodedServiceId%>');"><%=encodedServiceId%></a></td>
	<td><%=serviceStatus(bean.getAppContext(), serviceId)%></td>
</tr>
<%}}%>
</tbody>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAddService",  "services.add",  "services.addHint");
page_menu_button(session, out, "mbDelete", "services.delete", "services.deleteHint");
page_menu_space(out);
page_menu_button(session, out, "mbStartService",  "services.start",  "services.startHint");
page_menu_button(session, out, "mbStopService", "services.stop", "services.stopHint");
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
