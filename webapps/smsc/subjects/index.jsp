<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.subjects.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.subjects.Index"/>
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
		response.sendRedirect("subjectsFilter.jsp");
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("subjectsAdd.jsp");
		return;
	case Index.RESULT_EDIT:
		response.sendRedirect("subjectsEdit.jsp?name="+URLEncoder.encode(bean.getEditName()));
		return;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SUBJECTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>

<h1>Subjects</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editName>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<input type=hidden ID=jbutton value="jbutton">
<script>
function edit(name_to_edit)
{
	document.all.jbutton.name = "mbEdit";
	opForm.editName.value = name_to_edit;
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
<table class=rep0 cellspacing=1 width="100%">
<col width="1%">
<col width="60%" align=left>
<col width="20%" align=left>
<col width="20%" align=center>
<thead>
<tr>
	<th>&nbsp;</th>
	<th><a href="#" title="Sort by name" onclick='return setSort("Name")'>name</a></th>
	<th><a href="#" title="Sort by default SME" onclick='return setSort("Default SME")'>default SME</a></th>
	<th><a href="#" title="Sort by masks" onclick='return setSort("Masks")'>masks</a></th>
</tr>
</thead>
<tbody>
<%{
int row = 0;
for(Iterator i = bean.getSubjects().iterator(); i.hasNext(); row++)
{
DataItem item = (DataItem) i.next();
String name = (String)item.getValue("Name");
String defSme = (String)item.getValue("Default SME");
Vector masks = (Vector)item.getValue("Masks");
String encName = StringEncoderDecoder.encode(name);
String encDefSme = StringEncoderDecoder.encode(defSme);
%>
<tr class=row<%=row&1%>>
	<td class=check><input class=check type=checkbox name=checkedSubjects value="<%=encName%>" <%=bean.isSubjectChecked(name) ? "checked" : ""%>></td>
	<td class=name><a href="#" title="Edit subject" onClick='return edit("<%=encName%>")'><%=encName%></a></td>
	<td><%=encDefSme%></td>
	<td><%
	for (Iterator j = masks.iterator(); j.hasNext(); ) 
	{
		String mask = (String) j.next();
		String encMask = StringEncoderDecoder.encode(mask);
		out.print(encMask);
		if (j.hasNext())
			out.print("<br>");
	}%></td>
</tr>
<%}}%>
</tbody>
</table>
<div class=but0>
<input class=btn type=submit name=mbAdd value="Add subject" title="Add new subject">
<input class=btn type=submit name=mbDelete value="Delete subject(s)" title="Delete selected subject(s)">
<br>

<label title="First page"><%if (bean.isFirst()) {%>&lt;&lt;<%} else {%><a href="#" onclick='return navigate("mbFirst")'>&lt;&lt;</a><%}%></label>
<label title="Previous page"><%if (bean.isFirst()) {%>&lt;<%} else {%><a href="#" onclick='return navigate("mbPrev")'>&lt;</a><%}%></label>
<label title="Next page"><%if (bean.isLast()) {%>&gt;<%} else {%><a href="#" onclick='return navigate("mbNext")'>&gt;</a><%}%></label>
<label title="Last page"><%if (bean.isLast()) {%>&gt;&gt;<%} else {%><a href="#" onclick='return navigate("mbLast")'>&gt;&gt;</a><%}%></label>
<br>
<input class=btn type=submit name=mbFilter value="Filter" title="Filter subjects">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
