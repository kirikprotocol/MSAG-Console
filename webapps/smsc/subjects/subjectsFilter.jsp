<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsFilter,
                 ru.novosoft.smsc.jsp.util.tables.impl.SubjectFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case SubjectsFilter.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case SubjectsFilter.RESULT_OK:
		STATUS.append("Ok");
		break;
	case SubjectsFilter.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SUBJECTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<h1>Subjects filter</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<table class=frm0 cellspacing=0 width="100%">
<col width="10%" align=right>
<col width="85%">
<col width="10%">
<%int rowN = 0;%>
<tr>
	<th class=row<%=(rowN++)&1%> colspan="3">subjects</th>
</tr>
<%
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check id="subj_chk_<%=encName%>" type=checkbox name=checkedSubjects value="<%=encName%>" <%=bean.isSubjChecked(name) ? "checked" : ""%>></td>
	<td><label for="subj_chk_<%=encName%>"><%=encName%></label></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr>
	<th class=row<%=(rowN++)&1%> colspan="3">default SMEs:</th>
</tr>
<%
for (Iterator i = bean.getAllSmes().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check id="subj_sme_<%=encName%>" type=checkbox name=checkedSmes value="<%=encName%>" <%=bean.isSmeChecked(name) ? "checked" : ""%>></td>
	<td><label for="subj_sme_<%=encName%>"><%=encName%></label></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr>
	<th class=row<%=(rowN++)&1%> colspan="3">masks:</th>
</tr>
<%
for (int i=0; i<bean.getMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th>&nbsp;</th>
	<td><input class=txtW name=masks value="<%=bean.getMasks()[i]%>"></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr class=rowLast>
	<th>&nbsp;</th>
	<td><input class=txtW name=masks></td>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to subject"></td>
</tr>
</table>
<div class=but0>
<input class=btn type=submit name=mbApply value="Apply" title="Apply filter">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>