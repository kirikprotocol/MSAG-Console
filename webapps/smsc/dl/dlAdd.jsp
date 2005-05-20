<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*"%>
<jsp:useBean id="dl_dlAdd_bean" class="ru.novosoft.smsc.jsp.dl.dlAdd" scope="session"/>
<jsp:setProperty name="dl_dlAdd_bean" property="*"/>
<%
ru.novosoft.smsc.jsp.dl.dlAdd bean = dl_dlAdd_bean;
TITLE = getLocString("dl.addTitle");
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
page_menu_button(session, out, "mbSave",  "common.buttons.add",  "dl.addHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>