<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo,
					 ru.novosoft.smsc.admin.route.SmeStatus,
					 ru.novosoft.smsc.admin.route.SME"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><%@ include file = "/WEB-INF/inc/show_sme_status.jsp"%><%
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
System.out.println("*** connected_statuses.jsp called");
out.println();
for (Iterator i = c.iterator(); i.hasNext(); )
{
	String sId = (String) i.next();
	SmeStatus status = bean.getSmeStatus(sId);
	%><span <%=showSmeStatus(status)%><%
	if (i.hasNext())
		out.print(", ");
}
/*} catch (Throwable t)
{
	t.printStackTrace(new java.io.PrintWriter(out));
}*/
%>