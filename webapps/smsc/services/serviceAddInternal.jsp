<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.services.ServiceAddInternal"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.ServiceAddInternal"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("services.addTitle2");
switch(bean.process(request))
{
	case ServiceAddInternal.RESULT_DONE:
		response.sendRedirect("resourceGroups.jsp");
		return;
	case ServiceAddInternal.RESULT_OK:

		break;
	case ServiceAddInternal.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<%int rowN = 0;
page_menu_begin(out);
page_menu_button(session, out, "mbNext",  "common.buttons.finish",  "services.add");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "return clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><div class=content>
<div class=page_subtitle><%=getLocString("services.addSubTitle")%>:</div>
<table class=properties_list cellspacing=0 cellpadding=0>
<col width="15%" align=right>
<col width="85%">
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.systemId")%>:</th>
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
page_menu_button(session, out, "mbNext",  "common.buttons.finish",  "services.add");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "return clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
