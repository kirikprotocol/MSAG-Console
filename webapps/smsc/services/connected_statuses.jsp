<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><%
/*try
{*/
bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), new LinkedList(), request.getUserPrincipal());
List c = bean.getServiceIds();
for (Iterator i = c.iterator(); i.hasNext(); )
{
	out.print('"' + (String) i.next() + '"');
	if (i.hasNext())
		out.print(", ");
}
out.println();
for (Iterator i = c.iterator(); i.hasNext(); )
{
	String sId = (String) i.next();
	if (bean.isServiceConnected(sId))
	{
		%><span class=C080>connected</span><%
	}
	else
	{
		%><span class=C800>disconnected</span><%
	}
	if (i.hasNext())
		out.print(", ");
}
/*} catch (Throwable t)
{
	t.printStackTrace(new java.io.PrintWriter(out));
}*/
%>