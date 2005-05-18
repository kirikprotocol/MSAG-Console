<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.AliasesAdd"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.AliasesAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Add alias";
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
page_menu_button(session, out, "mbSave",  "Add alias",  "Add new alias");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="aliasBody.jsp"%><%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "Add alias",  "Add new alias");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@ 
include file="/WEB-INF/inc/html_3_footer.jsp"%><%@ 
include file="/WEB-INF/inc/code_footer.jsp"%>