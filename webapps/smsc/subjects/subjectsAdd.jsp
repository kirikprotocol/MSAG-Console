<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsAdd"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Add subject";
int beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal);
switch(beanResult)
{
	case SubjectsAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case SubjectsAdd.RESULT_OK:
		STATUS.append("Ok");
		break;
	case SubjectsAdd.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SUBJECTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%boolean isNew = true;%>
<%@ include file="subjectBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Add subject" title="Add new subject">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>