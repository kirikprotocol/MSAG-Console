<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Aliases";
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
		response.sendRedirect("aliasesFilter.jsp");
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("aliasesAdd.jsp");
		return;
	case Index.RESULT_EDIT:
		response.sendRedirect("aliasesEdit.jsp?alias="+URLEncoder.encode(bean.getEditAlias())
		                                     +"&address="+URLEncoder.encode(bean.getEditAddress())
		                                     +"&hide="+bean.isEditHide());
		return;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ALIASES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editAlias>
<input type=hidden name=editAddress>
<input type=hidden name=editHide>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>

<script>
function edit(alias, address, hide)
{
	document.all.jbutton.name = "mbEdit";
	opForm.editAlias.value = alias;
	opForm.editAddress.value = address;
	opForm.editHide.value = hide;
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
function navigate(direction)
{
	document.all.jbutton.name = direction;
	document.all.jbutton.value = direction;
	opForm.submit();
	return false;
}
</script>
<table class=secRep cellspacing=1 width="100%">
<col width="1%">
<col width="60%" align=left>
<col width="20%" align=left>
<col width="20%" align=center>
<thead>
<tr>
	<th>&nbsp;</th>
	<th><a href="#" <%=bean.getSort().endsWith("Alias") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by mask" onclick='return setSort("Alias")'>alias</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("Address") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by codepage" onclick='return setSort("Address")'>address</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("Hide") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by report info" onclick='return setSort("Hide")'>hide</a></th>
</tr>
</thead>
<tbody>
<%{
int row = 0;
for(Iterator i = bean.getAliases().iterator(); i.hasNext();)
{
DataItem item = (DataItem) i.next();
String encAlias = StringEncoderDecoder.encode((String)item.getValue("Alias"));
String encAddress = StringEncoderDecoder.encode((String)item.getValue("Address"));
%>
<tr class=row<%=(row++)&1%>>
	<td class=check><input class=check type=checkbox name=checkedAliases value="<%=encAlias%>" <%=bean.isAliasChecked((String)item.getValue("Alias")) ? "checked" : ""%>></td>
	<td class=name><a href="#" title="Edit alias" onClick='return edit("<%=encAlias%>", "<%=encAddress%>", "<%=item.getValue("Hide")%>")'><%=encAlias%></a></td>
	<td><%=encAddress%></td>
	<td><%=item.getValue("Hide")%></td>
</tr>
<%}}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbAdd value="Add alias" title="Add alias">
<input class=btn type=submit name=mbDelete value="Delete alias(es)" title="Delete selected aliases">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
