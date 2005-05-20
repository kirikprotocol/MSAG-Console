<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.AliasesAdd"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.AliasesAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("aliases.addTitle");
switch(bean.process(request))
{
	case AliasesAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ALIASES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%><%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "aliases.add",  "aliases.addHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="aliasBody.jsp"%><%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "aliases.add",  "aliases.addHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@ 
include file="/WEB-INF/inc/html_3_footer.jsp"%><%@ 
include file="/WEB-INF/inc/code_footer.jsp"%>