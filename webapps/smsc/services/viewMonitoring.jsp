<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="header.jsp"%>
<% 
	String name = request.getParameter("service");
	String host = request.getParameter("host");
	Map data = serviceManager.getServiceMonitoringData(name);
%>
	<table class="data" cellspacing="0">
		<tr class="data">
			<th class="data">Parameter name</th>
			<th class="data">Value</th>
		</tr>
<%
	for (Iterator i = data.keySet().iterator(); i.hasNext(); ) {
		String paramName = (String)i.next();
		%>
		<tr class="data">
			<td class="data"><%=paramName%></td>
			<td class="data"><%=data.get(paramName)%></td>
		</tr>
		<%
	}
%>
	</table>
	<%=action("Host \""+host+'"', "viewHost.jsp", "host", host)%>
<%@include file="footer.jsp"%>
