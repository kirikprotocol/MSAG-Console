<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesAdd,
                 ru.novosoft.smsc.admin.profiler.Profile"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Add profile";
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case ProfilesAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ProfilesAdd.RESULT_OK:
		STATUS.append("Ok");
		break;
	case ProfilesAdd.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
boolean isEdit = false;
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="profileBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Add profile" title="Add profile">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>