<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo,
                ru.novosoft.smsc.admin.Constants,
                ru.novosoft.smsc.util.Functions"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><%
bean.process(request);
out.print('"' + Functions.getServiceId(request.getServletPath()) + '"');
out.println();
switch (bean.getServiceStatus(Functions.getServiceId(request.getServletPath())))
{
	case ServiceInfo.STATUS_ONLINE1:
		%><span class=C080><%=getLocString("common.statuses.online1")%></span><%
		break;
	case ServiceInfo.STATUS_ONLINE2:
		%><span class=C008><%=getLocString("common.statuses.online2")%></span><%
		break;
	case ServiceInfo.STATUS_OFFLINE:
		%><span class=C800><%=getLocString("common.statuses.offline")%></span><%
		break;
	default:
		%><span class=C000>undefined</span><%
		break;
}
%>