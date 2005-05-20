<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.providers.ProvidersEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.providers.ProvidersEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("providers.editTitle");
switch (bean.process(request))
{
	case ProvidersEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ProvidersEdit.RESULT_OK:

		break;
	case ProvidersEdit.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROVIDERS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "providers.saveEditHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "providers.cancelEditHint", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<input type=hidden name="id" value="<%=bean.getId()%>">
<%@ include file="providerBody.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "providers.saveEditHint");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "providers.cancelEditHint", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>