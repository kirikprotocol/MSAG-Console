<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.Index"/>
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
		response.sendRedirect("aliasesFilter.jsp");
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("aliasesAdd.jsp");
		return;
	case Index.RESULT_EDIT:
		response.sendRedirect("aliasesEdit.jsp?alias="+URLEncoder.encode(bean.getEditAlias())
		                                     +"&address="+URLEncoder.encode(bean.getEditAddress())
		                                     +"&hide="+bean.getEditHide());
		return;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ALIASES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>

<h1>Profiles</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editAlias>
<input type=hidden name=editAddress>
<input type=hidden name=editHide>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<input type=hidden ID=jbutton value="jbutton">
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
</script>
<table class=rep0 cellspacing=1 width="100%">
<col width="1%">
<col width="60%" align=left>
<col width="20%" align=left>
<col width="20%" align=center>
<thead>
<tr>
	<th><a href="#" title="Sort by mask" onclick='return setSort("Alias")'>alias</a></th>
	<th><a href="#" title="Sort by codepage" onclick='return setSort("Address")'>address</a></th>
	<th><a href="#" title="Sort by report info" onclick='return setSort("Hide")'>hide</a></th>
</tr>
</thead>
<tbody>
<%{
int row = 0;
for(Iterator i = bean.getAliases().iterator(); i.hasNext(); row++)
{
DataItem item = (DataItem) i.next();
%>
<tr class=row<%=row&1%>>
	<td class=name><a href="#" title="Edit profile" onClick='return edit("<%=StringEncoderDecoder.encode((String)item.getValue("Alias"))%>", "<%=StringEncoderDecoder.encode((String)item.getValue("Address"))%>", "<%=item.getValue("Hide")%>")'><%=StringEncoderDecoder.encode((String)item.getValue("Alias"))%></a></td>
	<td><%=StringEncoderDecoder.encode((String)item.getValue("Address"))%></td>
	<td><%=item.getValue("Hide")%></td>
</tr>
<%}}%>
</tbody>
</table>
<div class=but0>
<input class=btn type=submit name=mbAdd value="Add alias" title="Add alias"><br>

<input class=btn type=submit name=mbFirst value="First" title="First page"<%=bean.isFirst() ? " disabled" : ""%>>
<input class=btn type=submit name=mbPrev value="Prev" title="Previous page"<%=bean.isFirst() ? " disabled" : ""%>>
<input class=btn type=submit name=mbNext value="Next" title="Next page"<%=bean.isLast() ? " disabled" : ""%>>
<input class=btn type=submit name=mbLast value="Last" title="Last page"<%=bean.isLast() ? " disabled" : ""%>><br>
<input class=btn type=submit name=mbFilter value="Filter" title="Filter profiles">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
