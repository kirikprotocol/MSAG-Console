<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.services.ServiceAddInternal"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.ServiceAddInternal"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Add service: step 2";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case ServiceAddInternal.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ServiceAddInternal.RESULT_OK:
		STATUS.append("Ok");
		break;
	case ServiceAddInternal.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<%int rowN = 0;
page_menu_begin(out);
page_menu_button(out, "mbNext",  "Finish",  "Add service");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><div class=content>
<div class=page_subtitle>Select service type:</div>
<table class=properties_list cellspacing=0 cellpadding=0>
<col width="15%" align=right>
<col width="85%">
<tr class=row<%=(rowN++)&1%>>
	<th>system id:</th>
	<td><input class=txt type="text" name="serviceId" value="<%=StringEncoderDecoder.encode(bean.getServiceId())%>"></td>
</tr>
<%@include file="serviceSmeBody.jsp"%>
</table>
<%if (bean.getHostName() != null)
{
%><input class=txt type=hidden name=hostName value="<%=bean.getHostName()%>"><%
}%>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbNext",  "Finish",  "Add service");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
