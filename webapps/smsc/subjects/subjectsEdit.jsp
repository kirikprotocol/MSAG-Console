<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Edit subject";
int beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal);
switch(beanResult)
{
	case SubjectsEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case SubjectsEdit.RESULT_OK:
		STATUS.append("Ok");
		break;
	case SubjectsEdit.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SUBJECTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%boolean isNew = false;%>
<%@ include file="subjectBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save subject" title="Apply changes">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>