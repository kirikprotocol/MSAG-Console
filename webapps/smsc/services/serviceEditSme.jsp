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
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<%int rowN = 0;%>
<input type=hidden name="serviceId" value="<%=StringEncoderDecoder.encode(bean.getServiceId())%>">

<div class=secInfo>Service parameters:</div>
<table class=secRep cellspacing=1 width="100%">
<%@include file="serviceSmeBody.jsp"%>
</table>

<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save" title="Save new service parameters">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
