<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");%>
<h2>Remove host <i><%=host%></i>: </h2>
Waiting for shutdown services:<br>
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
	<% } else if (host.equals("smsc_2")) { %>
		<tr class="list">
			<td class="list" colspan="2">нет сервисов</td>
		</tr>
	<% } else if (host.equals("smsc.again")) { %>
		<tr class="list">
			<td class="list" colspan="2">нет сервисов</td>
		</tr>
	<% } %>
</table>

<br>
<hr style="width: 100%;">
<br>

<font color=red>Services was not stopped in 10 seconds:<br>
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
	<% } else if (host.equals("smsc_2")) { %>
		<tr class="list">
			<td class="list" colspan="2">нет сервисов</td>
		</tr>
	<% } else if (host.equals("smsc.again")) { %>
		<tr class="list">
			<td class="list" colspan="2">нет сервисов</td>
		</tr>
	<% } %>
</table>
Do you want to 
<a href="remove_host_with_services_shutdown.jsp?host=<%=host%>">Wait more 10 seconds for shutdown services</a>
or
<a href="remove_host_with_services_kill.jsp?host=<%=host%>">kill all running services</a><br>
</font>
<%@ include file="footer.jsp"%>