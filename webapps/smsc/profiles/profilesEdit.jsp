<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesEdit,
                 ru.novosoft.smsc.admin.profiler.Profile,
					  java.net.URLEncoder"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("profiles.editTitle");
switch(bean.process(request))
{
	case ProfilesEdit.RESULT_DONE:
		if ("lookup".equals(bean.getReturnPath()))
			response.sendRedirect("lookup.jsp?profile=" + URLEncoder.encode(bean.getMask(), "UTF-8"));
		else if ("groups".equals(bean.getReturnPath()))
			response.sendRedirect("groups.jsp");
		else if ("profiles".equals(bean.getReturnPath()))
			response.sendRedirect("index.jsp" + "?queried=true");
		else
			response.sendRedirect("index.jsp" + "?queried=true");
		return;
	case ProfilesEdit.RESULT_OK:
		break;
	case ProfilesEdit.RESULT_ERROR:
		break;
}
boolean isEdit = true;
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "common.buttons.saveChanges");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "profiles.cancelEditHint", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<input type=hidden name=queried value="true">
<%@ include file="profileBody.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "common.buttons.saveChanges");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "profiles.cancelEditHint", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>