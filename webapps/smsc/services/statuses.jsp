<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><jsp:setProperty name="bean" property="*"/><%
/*try
{*/
String CPATH = request.getContextPath() + "/smsc";
bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), new LinkedList(), request.getUserPrincipal());
List c = new ArrayList(bean.getServiceIds());
if (!c.contains(ru.novosoft.smsc.admin.Constants.SMSC_SME_ID))
c.add(ru.novosoft.smsc.admin.Constants.SMSC_SME_ID);
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
	if (bean.isColored())
	{
		switch (bean.getServiceStatus(sId))
		{
			case ServiceInfo.STATUS_RUNNING:
				out.print("<img src=\"/images/ic_running.gif\" width=10 height=10 title='running'>");
				break;
			case ServiceInfo.STATUS_STOPPING:
				out.print("<img src=\"/images/ic_stopping.gif\" width=10 height=10 title='stopping'>");
				break;
			case ServiceInfo.STATUS_STOPPED:
				out.print("<img src=\"/images/ic_stopped.gif\" width=10 height=10 title='stopped'>");
				break;
			case ServiceInfo.STATUS_STARTING:
				out.print("<img src=\"/images/ic_starting.gif\" width=10 height=10 title='starting'>");
				break;
			default:
				out.print("<img src=\"/images/ic_unknown.gif\" width=10 height=10 title='unknown'>");
				break;
		}
	}
	else
	{
		switch (bean.getServiceStatus(sId))
		{
			case ServiceInfo.STATUS_RUNNING:
				out.print("running");
				break;
			case ServiceInfo.STATUS_STOPPING:
				out.print("stopping");
				break;
			case ServiceInfo.STATUS_STOPPED:
				out.print("stopped");
				break;
			case ServiceInfo.STATUS_STARTING:
				out.print("starting");
				break;
			default:
				out.print("unknown");
				break;
		}
	}
	if (i.hasNext())
		out.print(", ");
}
/*} catch (Throwable t)
{
	t.printStackTrace(new java.io.PrintWriter(out));
}*/
%>