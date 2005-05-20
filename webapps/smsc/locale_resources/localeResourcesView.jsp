<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesView,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 java.net.URLEncoder"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesView"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("locale.viewTitle");
switch(bean.process(request))
{
	case LocaleResourcesView.RESULT_OK:
		break;
	case LocaleResourcesView.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case LocaleResourcesView.RESULT_EDIT:
		response.sendRedirect("localeResourcesEdit.jsp?locale=" + URLEncoder.encode(bean.getLocale(), "UTF-8"));
		return;
	case LocaleResourcesView.RESULT_ERROR:
		break;
	default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_LOCALE_RESOURCES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
	page_menu_begin(out);
	page_menu_button(session, out, "mbEdit", "common.buttons.edit", "locale.editHint");
	page_menu_button(session, out, "mbDone", "common.buttons.done", "locale.doneHint");
	page_menu_space(out);
	page_menu_end(out);
%>
<div class=content>
<input type=hidden name=locale value="<%=bean.getLocale()%>">
<iframe width="100%" height="400px" src="<%=CPATH%>/locale_resources/localeResourcesResourceFile.jsp?locale=<%=URLEncoder.encode(bean.getLocale(), "UTF-8")%>"></iframe>
</div>
<%
	page_menu_begin(out);
	page_menu_button(session, out, "mbEdit", "common.buttons.edit", "locale.editHint");
	page_menu_button(session, out, "mbDone", "common.buttons.done", "locale.doneHint");
	page_menu_space(out);
	page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
