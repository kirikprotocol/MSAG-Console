<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo,
					 ru.novosoft.smsc.admin.route.SmeStatus,
					 ru.novosoft.smsc.admin.route.SME"%>
<%@ include file="/WEB-INF/inc/localization.jsp"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"
/><%!
String CPATH = "/smsc";
%><%@ include file = "/WEB-INF/inc/show_sme_status.jsp"
%><%
/*try
{*/
CPATH = request.getContextPath() + "/smsc";
bean.process(request);
List c = bean.getServiceIds();
for (Iterator i = c.iterator(); i.hasNext(); )
{
	out.print('"' + (String) i.next() + '"');
	if (i.hasNext())
		out.print(", ");
}
//System.out.println("*** connected_statuses.jsp called");
out.println();
for (Iterator i = c.iterator(); i.hasNext(); )
{
	String sId = (String) i.next();
	SmeStatus status = bean.getSmeStatus(sId);
	out.print("<span " + showSmeStatus(status));
	if (i.hasNext())
		out.print(", ");
}
/*} catch (Throwable t)
{
	t.printStackTrace(new java.io.PrintWriter(out));
}*/
%>