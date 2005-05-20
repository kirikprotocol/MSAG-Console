<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.RoutesAdd"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.RoutesAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("routes.addTitle");
switch(bean.process(request))
{
	case RoutesAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case RoutesAdd.RESULT_OK:
		break;
	case RoutesAdd.RESULT_ERROR:
		break;
}

%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ROUTES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "routes.add",  "routes.addHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="routeBody.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "routes.add",  "routes.addHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>