<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo,
                ru.novosoft.smsc.admin.Constants,
                ru.novosoft.smsc.util.Functions"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><%
bean.process(request);
out.print('"' + Functions.getServiceId(request.getServletPath()) + '"');
out.println();
switch (bean.getServiceStatus(Functions.getServiceId(request.getServletPath())))
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