<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo,
					 ru.novosoft.smsc.admin.route.SmeStatus,
					 ru.novosoft.smsc.admin.route.SME"%><%@
 include file="/WEB-INF/inc/localization.jsp"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><%!
String CPATH = "/smsc";%><%@
 include file = "/WEB-INF/inc/show_sme_status.jsp"%><%
CPATH = request.getContextPath() + "/smsc";
bean.process(request);
List c = bean.getServiceIds();
String result = "";
for (Iterator i = c.iterator(); i.hasNext(); )
{
	result += '"' + (String) i.next() + '"';
	if (i.hasNext()) result += ", ";
}
result += "\r\n";
for (Iterator i = c.iterator(); i.hasNext(); )
{
	String sId = (String) i.next();
	SmeStatus status = bean.getSmeStatus(sId);
    result += "<span " + showSmeStatus(status);
	if (i.hasNext()) result += ", ";
}
out.print(result);%>