<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsFilter,
					  ru.novosoft.smsc.jsp.util.tables.impl.subject.SubjectFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Filter subjects";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
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
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter", "clickClear()");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Default SMEs ~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>Default SMEs</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width=1%>
<col width=99%>
<%
rowN = 0;
for (Iterator i = bean.getAllSmes().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check id="subj_sme_<%=encName%>" type=checkbox name=checkedSmes value="<%=encName%>" <%=bean.isSmeChecked(name) ? "checked" : ""%>></td>
	<td nowrap><label for="subj_sme_<%=encName%>"><%=encName%></label></td>
</tr>
<%}%>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Subjects ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>Subjects</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width=1%>
<col width=99%>
<%
rowN = 0;
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check id="subj_chk_<%=encName%>" type=checkbox name=checkedSubjects value="<%=encName%>" <%=bean.isSubjChecked(name) ? "checked" : ""%>></td>
	<td nowrap><label for="subj_chk_<%=encName%>"><%=encName%></label></td>
</tr>
<%}%>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Masks ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>Masks</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width=1%>
<col width=99%>
<%
rowN = 0;
for (int i=0; i<bean.getMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<td colspan=2><input class=txt name=masks value="<%=bean.getMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=rowLast>
	<td><input class=txt name=masks validation="mask" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAdd", "Add", "Add new mask to subjects filter");%></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter", "clickClear()");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>