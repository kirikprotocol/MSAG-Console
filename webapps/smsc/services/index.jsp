<%@include file="header.jsp"%>
	<h1>Hosts:</h1>
	
	<table class="list" cellspacing="0">
		<tr class="list">
			<th class="list">name</th>
			<th  class="list" colspan=2>actions</th>
		</tr>
		<%
		try {
			Set names = serviceManager.getHosts();
			for (Iterator i = names.iterator(); i.hasNext(); ) {
				String name = (String)i.next();
				%>
				<tr class="list">
					<td class="list"><%=action(name, "viewHost.jsp", "host", name)%></td>
					<td class="list"><%=action("view", "viewHost.jsp", "host", name)%></td>
					<td class="list"><%=action("remove(disconnect)", "removeHost.jsp", "host", name)%></td>
				</tr><%
			}
		} catch (Throwable t) {
			t.printStackTrace();
			%><pre style="color=red"><%=t.toString()%></pre><%
		}
		%>
	</table>
	<form method="post" action="addHost.jsp">
		<table border=1px>
			<tr><th>host</th><td><input type="Text" name="host"></td></tr>
			<tr><th>port</th><td><input type="Text" name="port"></td></tr>
			<tr><td colspan="2"><input type="Submit" value="add host"></td></tr>
		</table>
	</form>
	<%=action("Refresh services list on all hosts", "refreshServices.jsp")%>
<%@include file="footer.jsp"%>
