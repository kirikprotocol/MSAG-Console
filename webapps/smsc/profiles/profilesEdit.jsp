<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesEdit,
                 ru.novosoft.smsc.admin.profiler.Profile"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Edit profile";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case ProfilesEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ProfilesEdit.RESULT_OK:
		STATUS.append("Ok");
		break;
	case ProfilesEdit.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
boolean isEdit = true;
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="profileBody.jsp"%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save" title="Save changes">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>