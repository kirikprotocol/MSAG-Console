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
	if (bean.isColored())
	{
		switch (bean.getServiceStatus(sId))
		{
			case ServiceInfo.STATUS_RUNNING:
                result += "<img src=\"/images/ic_running.gif\" width=10 height=10 title='" + getLocString("common.statuses.running") + "'>";
				break;
			case ServiceInfo.STATUS_STOPPING:
				result += "<img src=\"/images/ic_stopping.gif\" width=10 height=10 title='" + getLocString("common.statuses.stopping") + "'>";
				break;
			case ServiceInfo.STATUS_STOPPED:
				result += "<img src=\"/images/ic_stopped.gif\" width=10 height=10 title='" + getLocString("common.statuses.stopped") + "'>";
				break;
			case ServiceInfo.STATUS_STARTING:
				result += "<img src=\"/images/ic_starting.gif\" width=10 height=10 title='" + getLocString("common.statuses.starting") + "'>";
				break;
			default:
				result += "<img src=\"/images/ic_unknown.gif\" width=10 height=10 title='" + getLocString("common.statuses.unknown") + "'>";
				break;
		}
	}
	else
	{
		switch (bean.getServiceStatus(sId))
		{
			case ServiceInfo.STATUS_RUNNING:
				result += getLocString("common.statuses.running");
				break;
			case ServiceInfo.STATUS_STOPPING:
				result += getLocString("common.statuses.stopping");
				break;
			case ServiceInfo.STATUS_STOPPED:
				result += getLocString("common.statuses.stopped");
				break;
			case ServiceInfo.STATUS_STARTING:
				result += getLocString("common.statuses.starting");
				break;
			default:
				result += getLocString("common.statuses.unknown");
				break;
		}
	}
	if (i.hasNext()) result += ", ";
}
out.print(result);%>