<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo,
					 ru.novosoft.smsc.admin.Constants"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><%
bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), new LinkedList(), request.getUserPrincipal());
out.print('"' + Constants.SMSC_SME_ID + '"');
out.println();
switch (bean.getServiceStatus(Constants.SMSC_SME_ID))
{
	case ServiceInfo.STATUS_RUNNING:
		%><span class=C080>running</span><%
		break;
	case ServiceInfo.STATUS_STOPPING:
		%><span class=C008>stopping</span><%
		break;
	case ServiceInfo.STATUS_STOPPED:
		%><span class=C800>stopped</span><%
		break;
	case ServiceInfo.STATUS_STARTING:
		%><span class=C0F0>starting</span><%
		break;
	default:
		%><span class=C000>undefined</span><%
		break;
}
%>