<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem,
				 ru.novosoft.smsc.admin.route.SourceList,
				 ru.novosoft.smsc.admin.route.DestinationList
"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Routes";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
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
	case Index.RESULT_FILTER:
		response.sendRedirect("routesFilter.jsp");
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("routesAdd.jsp");
		return;
	case Index.RESULT_EDIT:
		response.sendRedirect("routesEdit.jsp?routeId="+URLEncoder.encode(bean.getEditRouteId()));
		return;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ROUTES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
 

<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editRouteId>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>

<script>
function edit(name_to_edit)
{
	document.all.jbutton.name = "mbEdit";
	opForm.editRouteId.value = name_to_edit;
	opForm.submit();
	return false;
}
function setSort(sorting)
{
	if (sorting == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = sorting;
	opForm.submit();
	return false;
}
</script>
<table class=secRep cellspacing=1 width="100%">
<col width="1%">
<col width="60%" align=left>
<col width="20%" align=center>
<col width="20%" align=center>
<col width="20%" align=center>
<col width="20%" align=center>
<col width="20%" align=center>
<thead>
<tr>
	<th class=ico><img src="<%=CPATH%>/img/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th><a href="#" <%=bean.getSort().endsWith("Route ID")                ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by name" onclick='return setSort("Route ID")'                        >name</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("active")                  ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by active status" onclick='return setSort("active")'                 >active</a></th>

	<th><a href="#" <%=bean.getSort().endsWith("isEnabling")              ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by active status" onclick='return setSort("isEnabling")'             >allow</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("isBilling")               ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by active status" onclick='return setSort("isBilling")'              >billing</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("isArchiving")             ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by active status" onclick='return setSort("isArchiving")'            >archiving</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("suppressDeliveryReports") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by active status" onclick='return setSort("suppressDeliveryReports")'>reports</a></th>
</tr>
</thead>
<tbody>
<%{
int row = 0;
for(Iterator i = bean.getRoutes().iterator(); i.hasNext(); row++)
{
DataItem item = (DataItem) i.next();
String routeId = (String) item.getValue("Route ID");
SourceList sources = (SourceList) item.getValue("sources");
DestinationList destinations = (DestinationList) item.getValue("destinations");
boolean isActive                  = ((Boolean)item.getValue("active"                 )).booleanValue();
boolean isEnabling                = ((Boolean)item.getValue("isEnabling"             )).booleanValue();
boolean isBilling                 = ((Boolean)item.getValue("isBilling"              )).booleanValue();
boolean isArchiving               = ((Boolean)item.getValue("isArchiving"            )).booleanValue();
boolean isSuppressDeliveryReports = ((Boolean)item.getValue("suppressDeliveryReports")).booleanValue();

String encRouteId = StringEncoderDecoder.encode(routeId);
%>
<tr class=row<%=row&1%>>
	<td class=check><input class=check type=checkbox name=checkedRouteIds value="<%=encRouteId%>" <%=bean.isRouteChecked(routeId) ? "checked" : ""%>></td>
	<td class=name><a href="#" title="Edit route" onClick='return edit("<%=encRouteId%>")'><%=encRouteId%></a></td>
	<td class=check><input class=check type=checkbox disabled <%=isActive                  ? "checked" : ""%>></td>
	<td class=check><input class=check type=checkbox disabled <%=isEnabling                ? "checked" : ""%>></td>
	<td class=check><input class=check type=checkbox disabled <%=isBilling                 ? "checked" : ""%>></td>
	<td class=check><input class=check type=checkbox disabled <%=isArchiving               ? "checked" : ""%>></td>
	<td class=check><input class=check type=checkbox disabled <%=isSuppressDeliveryReports ? "checked" : ""%>></td>
</tr>
<%}}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbAdd value="Add route" title="Add new route">
<input class=btn type=submit name=mbDelete value="Delete route(s)" title="Delete selected route(s)">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
