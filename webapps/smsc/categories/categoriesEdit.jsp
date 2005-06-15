<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.providers.ProvidersEdit,
                 ru.novosoft.smsc.jsp.smsc.categories.CategoriesEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.categories.CategoriesEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("categories.editTitle");
switch (bean.process(request))
{
	case CategoriesEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case CategoriesEdit.RESULT_OK:

		break;
	case CategoriesEdit.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_CATEGORIES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "categories.saveEditHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "categories.cancelEditHint", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<input type=hidden name="id" value="<%=bean.getId()%>">
<%@ include file="categoryBody.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "categories.saveEditHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "categories.cancelEditHint", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>