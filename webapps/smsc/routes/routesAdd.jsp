<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.RoutesAdd"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.RoutesAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Add route";
switch(bean.process(appContext, errorMessages, request.getParameterMap(), loginedUserPrincipal))
{
	case RoutesAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case RoutesAdd.RESULT_OK:
		STATUS.append("Ok");
		break;
	case RoutesAdd.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}

%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ROUTES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"
%><%@ include file="routeBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Add route" title="Add new route">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>