<%@ page errorPage="error.jsp" %>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="../header.jsp"%>
<% 
	ServiceManager manager = ctx.getServiceManager();
	Map data = manager.getServiceMonitoringData(request.getParameter("service"));
%>
	<table class="data" cellspacing="0">
		<tr class="data">
			<th class="data">Parameter name</th>
			<th class="data">Value</th>
		</tr>
<%
	for (Iterator i = data.keySet().iterator(); i.hasNext(); ) {
		String name = (String)i.next();
		%>
		<tr class="data">
			<td class="data"><%=name%></td>
			<td class="data"><%=data.get(name)%></td>
		</tr>
		<%
	}
%>
	</table>
	<a href="index.jsp">Services</a>
<%@include file="../footer.jsp"%>
