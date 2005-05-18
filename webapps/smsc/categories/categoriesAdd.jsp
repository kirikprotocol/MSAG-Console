<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.providers.ProvidersAdd"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.categories.CategoriesAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Add Category";
switch (bean.process(request))
{
	case ProvidersAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ProvidersAdd.RESULT_OK:

		break;
	case ProvidersAdd.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_CATEGORIES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "Save",  "Save new category info");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel category adding", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="categoryBody.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "Save",  "Save new category info");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel category adding", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>