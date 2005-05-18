<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.providers.ProvidersEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.categories.CategoriesEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Edit Category";
switch (bean.process(request))
{
	case ProvidersEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ProvidersEdit.RESULT_OK:

		break;
	case ProvidersEdit.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_CATEGORIES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "Save",  "Save category info");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel category editing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<input type=hidden name="id" value="<%=bean.getId()%>">
<%@ include file="categoryBody.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "Save",  "Save category info");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel category editing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>