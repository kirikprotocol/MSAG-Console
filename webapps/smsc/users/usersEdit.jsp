<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.users.UsersEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.users.UsersEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Edit user";
switch (bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case UsersEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case UsersEdit.RESULT_OK:
		STATUS.append("Ok");
		break;
	case UsersEdit.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_USERS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="userBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save" title="Save alias">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>