<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem,
				 ru.novosoft.smsc.admin.route.SourceList,
				 ru.novosoft.smsc.admin.route.DestinationList
"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
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
<h1>Routes</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editRouteId>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<input type=hidden ID=jbutton value="jbutton">
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
<table class=rep0 cellspacing=1 width="100%">
<col width="1%">
<col width="60%" align=left>
<col width="20%" align=left>
<col width="20%" align=center>
<thead>
<tr>
	<th>&nbsp;</th>
	<th><a href="#" title="Sort by name" onclick='return setSort("Route ID")'>name</a></th>
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
boolean isEnabling  = ((Boolean)item.getValue("isEnabling" )).booleanValue();
boolean isBilling   = ((Boolean)item.getValue("isBilling"  )).booleanValue();
boolean isArchiving = ((Boolean)item.getValue("isArchiving")).booleanValue();

String encRouteId = StringEncoderDecoder.encode(routeId);
%>
<tr class=row<%=row&1%>>
	<td class=check><input class=check type=checkbox name=checkedRouteIds value="<%=encRouteId%>" <%=bean.isRouteChecked(routeId) ? "checked" : ""%>></td>
	<td class=name><a href="#" title="Edit route" onClick='return edit("<%=encRouteId%>")'><%=encRouteId%></a></td>
</tr>
<%}}%>
</tbody>
</table>
<div class=but0>
<input class=btn type=submit name=mbAdd value="Add route" title="Add new route">
<input class=btn type=submit name=mbDelete value="Delete route(s)" title="Delete selected route(s)">
<br>

<input class=btn type=submit name=mbFirst value="First" title="First page"<%=bean.isFirst() ? " disabled" : ""%>>
<input class=btn type=submit name=mbPrev value="Prev" title="Previous page"<%=bean.isFirst() ? " disabled" : ""%>>
<input class=btn type=submit name=mbNext value="Next" title="Next page"<%=bean.isLast() ? " disabled" : ""%>>
<input class=btn type=submit name=mbLast value="Last" title="Last page"<%=bean.isLast() ? " disabled" : ""%>><br>
<input class=btn type=submit name=mbFilter value="Filter" title="Filter routes">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
