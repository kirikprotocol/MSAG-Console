<%@ include file="header.jsp"%>
<% String host = request.getParameter("host") == null ? "smsc" : request.getParameter("host");%>
<h2>Remove host <i><%=host%></i>: </h2>
Host <i><%=host%></i> contains running services:<br>
<table class="list" cellspacing="0">
	<tr class="list">
		<th class="list">Service Name</th>
		<th class="list">Status</th>
	</tr>
	<% if (host.equals("smsc")) { %>
		<tr class="list">
			<td class="list">Example service 1</td>
			<td class="list">running</td>
		</tr>
		<tr class="list">
			<td class="list"><font color="#888888">Third example service</font></td>
			<td class="list"><font color="#888888">stopped</font></td>
		</tr>
	<% } else if (host.equals("smsc_2")) { %>
		<tr class="list">
			<td class="list" colspan="2">нет сервисов</td>
		</tr>
	<% } else if (host.equals("smsc.again")) { %>
		<tr class="list">
			<td class="list"><font color="#888888">Second example service</font></td>
			<td class="list"><font color="#888888">stopped</font></td>
		</tr>
	<% } %>
</table>
Do you really want to shutdown all running services from this host and then remove this host?<br>
<a href="remove_host_with_services_2.jsp?host=<%=host%>">Yes, I want to shutdown services on this host and then remove host</a><br>
<a href="hosts.jsp">No, I don't</a>
<%@ include file="footer.jsp"%>