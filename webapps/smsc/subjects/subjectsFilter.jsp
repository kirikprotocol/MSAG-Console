<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsFilter,
                 ru.novosoft.smsc.jsp.util.tables.impl.SubjectFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Filter subjects";
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
<%int rowN = 0;%>
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Default SMEs ~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=secList>Default SMEs</div></td></tr>
<%
rowN = 0;
for (Iterator i = bean.getAllSmes().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td wrap><input class=check id="subj_sme_<%=encName%>" type=checkbox name=checkedSmes value="<%=encName%>" <%=bean.isSmeChecked(name) ? "checked" : ""%>>&nbsp;<label for="subj_sme_<%=encName%>"><%=encName%></label></td>
</tr>
<%}%>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Subjects ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=secList>Subjects</div></td></tr>
<%
rowN = 0;
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td nowrap><input class=check id="subj_chk_<%=encName%>" type=checkbox name=checkedSubjects value="<%=encName%>" <%=bean.isSubjChecked(name) ? "checked" : ""%>>&nbsp;<label for="subj_chk_<%=encName%>"><%=encName%></label></td>
</tr>
<%}%>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Masks ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=secList>Masks</div></td></tr>
<%
rowN = 0;
for (int i=0; i<bean.getMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td><input class=txtW name=masks value="<%=bean.getMasks()[i]%>"></td>
</tr>
<%}%>
<tr class=rowLast>
	<th class=label><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to subject"></th>
	<td><input class=txtW name=masks></td>
</tr>
</table>
<div class=secButtons>
<input class=btn type=submit name=mbApply value="Apply" title="Apply filter">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>