<%@ include file="/WEB-INF/inc/code_header.jsp"
%><%@ page import="ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesAdd,
                   ru.novosoft.util.jsp.MultipartServletRequest"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesAdd"
/><%
TITLE = "Add Resource";
FORM_URI = CPATH+"/upload";
FORM_METHOD = "POST";
FORM_ENCTYPE = "multipart/form-data";
MENU0_SELECTION = "MENU0_LOCALE_RESOURCES";
MultipartServletRequest multi = (MultipartServletRequest)request.getAttribute("multipart.request");
if (multi != null)
	request = multi;
%><jsp:setProperty name="bean" property="*"/><%
switch(bean.process(multi, appContext, errorMessages, loginedUserPrincipal))
{
	case LocaleResourcesAdd.RESULT_DONE:
		response.sendRedirect(CPATH+"/locale_resources/index.jsp");
		return;
	case LocaleResourcesAdd.RESULT_OK:
		STATUS.append("Ok");
		break;
	case LocaleResourcesAdd.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<input type="hidden" name="jsp" value="/locale_resources/localeResourcesAdd.jsp">
Resource file: <input type="file" name="resourceFile" accept="text/xml" class="txtW">
<div class=secButtons>
<input class=btn type=submit name=mbUpload value="Upload" title="Upload selected resource file">
<input class=btn type=submit name=mbCancel value="Cancel" title="Return to locales list without uploading resources">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
