<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.RoutesEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.RoutesEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Edit route";
switch(bean.process(appContext, errorMessages, request.getParameterMap(), loginedUserPrincipal))
{
	case RoutesEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case RoutesEdit.RESULT_OK:
		STATUS.append("Ok");
		break;
	case RoutesEdit.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}

%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ROUTES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<input type=hidden name=oldRouteId value="<%=bean.getOldRouteId()%>">
<%@ include file="routeBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save" title="Save changes">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="return clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>