<%@ include file="/WEB-INF/inc/code_header.jsp"
%><%@ page import="ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesAdd,
                   ru.novosoft.util.jsp.MultipartServletRequest,
                   ru.novosoft.smsc.jsp.SMSCJspException,
                   ru.novosoft.smsc.jsp.SMSCErrors,
                   ru.novosoft.smsc.admin.Constants"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesAdd"
/><%
TITLE = "Add Resource";
FORM_URI = CPATH+"/upload";
FORM_METHOD = "POST";
FORM_ENCTYPE = "multipart/form-data";
MENU0_SELECTION = "MENU0_LOCALE_RESOURCES";
%><jsp:setProperty name="bean" property="*"/><%
switch(bean.process(request))
{
	case LocaleResourcesAdd.RESULT_DONE:
		response.sendRedirect(CPATH+"/locale_resources/index.jsp");
		return;
	case LocaleResourcesAdd.RESULT_OK:
		break;
	case LocaleResourcesAdd.RESULT_ERROR:
		break;
	default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbUpload",  "Upload",  "Upload selected resource file");
page_menu_button(out, "mbCancel", "Cancel", "Return to locales list without uploading resources");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<input type="hidden" name="jsp" value="/locale_resources/localeResourcesAdd.jsp">
Resource file: <input class=txt type="file" name="resourceFile" accept="text/xml" class="txt">
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbUpload",  "Upload",  "Upload selected resource file");
page_menu_button(out, "mbCancel", "Cancel", "Return to locales list without uploading resources");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
