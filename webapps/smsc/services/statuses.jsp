<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><jsp:setProperty name="bean" property="*"/><%
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
	switch (bean.getServiceStatus(sId))
	{
		case ServiceInfo.STATUS_RUNNING:
			out.print(bean.isColored() ? "<span class=C080>" : "");
			out.print("running");
			out.print(bean.isColored() ? "</span>" : "");
			break;
		case ServiceInfo.STATUS_STOPPING:
			out.print(bean.isColored() ? "<span class=C008>" : "");
			out.print("stopping");
			out.print(bean.isColored() ? "</span>" : "");
			break;
		case ServiceInfo.STATUS_STOPPED:
			out.print(bean.isColored() ? "<span class=C800>" : "");
			out.print("stopped");
			out.print(bean.isColored() ? "</span>" : "");
			break;
		case ServiceInfo.STATUS_STARTING:
			out.print(bean.isColored() ? "<span class=C0F0>" : "");
			out.print("starting");
			out.print(bean.isColored() ? "</span>" : "");
			break;
		default:
			out.print(bean.isColored() ? "<span class=C000>" : "");
			out.print("undefined");
			out.print(bean.isColored() ? "</span>" : "");
			break;
	}
	if (i.hasNext())
		out.print(", ");
}
/*} catch (Throwable t)
{
	t.printStackTrace(new java.io.PrintWriter(out));
}*/
%>