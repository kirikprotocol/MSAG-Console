<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.AliasesAdd"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.AliasesAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Add alias";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case AliasesAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case AliasesAdd.RESULT_OK:
		STATUS.append("Ok");
		break;
	case AliasesAdd.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ALIASES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="aliasBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Add alias" title="Add new alias">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>