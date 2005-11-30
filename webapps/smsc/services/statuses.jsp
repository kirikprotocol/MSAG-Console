<%@page import="java.util.*, ru.novosoft.smsc.admin.service.ServiceInfo"%><%@
 include file="/WEB-INF/inc/localization.jsp"%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.services.Statuses"/><jsp:setProperty name="bean" property="*"/><%
String CPATH = request.getContextPath() + "/smsc";
bean.process(request);
List c = new ArrayList(bean.getServiceIds());
if (!c.contains(ru.novosoft.smsc.admin.Constants.SMSC_SME_ID))
  c.add(ru.novosoft.smsc.admin.Constants.SMSC_SME_ID);
if (!c.contains(ru.novosoft.smsc.admin.Constants.ARCHIVE_DAEMON_SVC_ID))
  c.add(ru.novosoft.smsc.admin.Constants.ARCHIVE_DAEMON_SVC_ID);
String result = "";
for (Iterator i = c.iterator(); i.hasNext(); )
{
    result += (String) i.next();
	if (i.hasNext()) result += ", ";
}
result += "\r\n";
for (Iterator i = c.iterator(); i.hasNext(); )
{
	String sId = (String) i.next();
	switch (bean.getServiceStatus(sId))
	{
		case ServiceInfo.STATUS_ONLINE:
			result += getLocString("common.statuses.online");
			break;
		case ServiceInfo.STATUS_OFFLINE:
			result += getLocString("common.statuses.offline");
			break;
		default:
			result += getLocString("common.statuses.unknown");
			break;
	}
	if (i.hasNext()) result += ", ";
}
out.print(result);%>