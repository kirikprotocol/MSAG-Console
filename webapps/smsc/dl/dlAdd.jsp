<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.dl.dlAdd" scope="session"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Create distribution list";
MENU0_SELECTION = "MENU0_DL";
final boolean isEditing = false;

switch (bean.process(appContext, errorMessages))
{
	case dlAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case dlAdd.RESULT_OK:
		STATUS.append("Ok");
		break;
	case dlAdd.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="dlBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save" title="Save new Distribution list">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>