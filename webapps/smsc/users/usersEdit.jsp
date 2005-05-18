<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.users.UsersEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.users.UsersEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Edit user";
switch (bean.process(request))
{
	case UsersEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case UsersEdit.RESULT_OK:

		break;
	case UsersEdit.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_USERS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "Save",  "Save user info");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel user editing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="userBody.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "Save",  "Save user info");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel user editing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>