<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.services.ServiceEditSme"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.ServiceEditSme"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Edit service \"" + bean.getServiceId() +"\" parameters";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case ServiceEditSme.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ServiceEditSme.RESULT_OK:
		STATUS.append("Ok");
		break;
	case ServiceEditSme.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
MENU0_SELECTION = "MENU0_SERVICES";
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Save",  "Save new service parameters");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<%int rowN = 0;%>
<input type=hidden name="serviceId" value="<%=StringEncoderDecoder.encode(bean.getServiceId())%>">

<div class=page_subtitle>Service parameters:</div>
<table class=properties_list cellspacing=0>
<%@include file="serviceSmeBody.jsp"%>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Save",  "Save new service parameters");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
