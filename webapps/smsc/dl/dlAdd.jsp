<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*"%>
<jsp:useBean id="dl_dlAdd_bean" class="ru.novosoft.smsc.jsp.dl.dlAdd" scope="session"/>
<jsp:setProperty name="dl_dlAdd_bean" property="*"/>
<%
ru.novosoft.smsc.jsp.dl.dlAdd bean = dl_dlAdd_bean;
TITLE = "Create distribution list";
MENU0_SELECTION = "MENU0_DL";
final boolean isEditing = false;

switch (bean.process(request))
{
	case dlAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="dlBody.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "Add",  "Add new Distribution list");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>