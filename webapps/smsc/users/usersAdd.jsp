<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.users.UsersAdd"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.users.UsersAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("users.addTitle");
switch (bean.process(request))
{
	case UsersAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case UsersAdd.RESULT_OK:

		break;
	case UsersAdd.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_USERS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save", "users.saveAddHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "users.cancelAddHint", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="userBody.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "users.saveAddHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "users.cancelAddHint", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>