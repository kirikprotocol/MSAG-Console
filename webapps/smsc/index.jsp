<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
%><%@ page import="ru.novosoft.smsc.jsp.smsc.Index"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
if (request.isUserInRole("super-admin"))
{
TITLE = getLocString("home.confStatusTitle");
}

switch(bean.process(request))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		break;
	case Index.RESULT_ERROR:
		break;
}
MENU0_SELECTION = "MENU0_NONE";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<br>
<br>
<br>
<br>
<br>
<%
page_menu_begin(out);
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>