<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesView"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesView"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Resource view";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case LocaleResourcesView.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case LocaleResourcesView.RESULT_OK:
		STATUS.append("Ok");
		break;
	case LocaleResourcesView.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_LOCALE_RESOURCES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<input type=hidden name=locale value="<%=bean.getLocale()%>"> 

<iframe width="100%" height="400px" src="localeResourcesResourceFile.jsp?locale=<%=URLEncoder.encode(bean.getLocale())%>"></iframe>

<div class=secButtons>
<input class=btn type=submit name=mbDone value="Done" title="Return to locales list">
</div>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
