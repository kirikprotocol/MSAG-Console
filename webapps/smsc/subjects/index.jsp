<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.subjects.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.subjects.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Subjects";
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

 

<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editName>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>

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
<table class=secRep cellspacing=1 width="100%">
<col width="1%">
<col width="60%" align=left>
<col width="20%" align=left>
<col width="20%" align=left>
<thead>
<tr>
	<th>&nbsp;</th>
	<th><a href="#" <%=bean.getSort().endsWith("Name") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by name" onclick='return setSort("Name")'>name</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("Default SME") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by default SME" onclick='return setSort("Default SME")'>default SME</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("Masks") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by masks" onclick='return setSort("Masks")'>masks</a></th>
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
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbAdd value="Add subject" title="Add new subject">
<input class=btn type=submit name=mbDelete value="Delete subject(s)" title="Delete selected subject(s)">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
