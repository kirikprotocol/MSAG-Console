<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.smsc_service.Index,
                 ru.novosoft.smsc.admin.service.ServiceInfo"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.smsc_service.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "SMSC";
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		STATUS.append("Ok");
		break;
	case Index.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SMSC";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
SMSC is <%
switch (bean.getStatus())
{
	case ServiceInfo.STATUS_RUNNING: {%><span style="color: #006600;">running</span><%break;}
	case ServiceInfo.STATUS_STARTING: {%><span style="color: #000099;">starting</span><%break;}
	case ServiceInfo.STATUS_STOPPING: {%><span style="color: #999900;">stopping</span><%break;}
	case ServiceInfo.STATUS_STOPPED: {%><span style="color: #FF0000;">stopped</span><%break;}
	case ServiceInfo.STATUS_UNKNOWN: {%><span style="color: White; background-color: Red;">unknown</span><%break;}
}
%>
<div class=secButtons>
<input class=btn type=submit name=mbStart value="Start" title="Start SMSC" <%=bean.getStatus() != ServiceInfo.STATUS_STOPPED ? "disabled" : ""%>>
<input class=btn type=submit name=mbStop value="Stop" title="Stop  SMSC" <%=bean.getStatus() != ServiceInfo.STATUS_RUNNING ? "disabled" : ""%>>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

