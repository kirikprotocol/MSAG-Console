<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.ServiceStatus"/><jsp:setProperty name="bean" property="*"/><%
bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), new LinkedList());
Collection c = bean.getServiceIds();
for (Iterator i = c.iterator(); i.hasNext(); )
{
	out.print('"' + (String) i.next() + '"');
	if (i.hasNext())
		out.print(", ");
}
out.println();
for (Iterator i = c.iterator(); i.hasNext(); )
{
	switch (bean.getServiceStatus((String) i.next()))
	{
		case ServiceInfo.STATUS_RUNNING:
			out.print("<span class=C080>running</span>");
			break;
		case ServiceInfo.STATUS_STOPPING:
			out.print("<span class=C008>stopping</span>");
			break;
		case ServiceInfo.STATUS_STOPPED:
			out.print("<span class=C800>stopped</span>");
			break;
		case ServiceInfo.STATUS_STARTING:
			out.print("<span class=C0F0>starting</span>");
			break;
		default:
			out.print("<span class=C000>undefined</span>");
			break;
	}
	if (i.hasNext())
		out.print(", ");
}
%>