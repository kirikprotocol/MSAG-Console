<%@ page errorPage="error.jsp" %>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="../header.jsp"%>
	<table class="list" cellspacing="0">
		<tr class="list">
			<th class="list">name</th>
			<th  class="list" colspan=4>actions</th>
		</tr>
		<%
		try {
			ServiceManager man = ctx.getServiceManager();
			Set names = man.getServiceNames();
			for (Iterator i = names.iterator(); i.hasNext(); ) {
				String name = (String)i.next();
				%><tr class="list">
					<td class="list"><%=name%></td>
					<td class="list"><a href="editServiceConfig.jsp?service=<%=name%>">edit config</a></td>
					<td class="list"><a href="viewServiceLogs.jsp?service=<%=name%>">view logs</a></td>
					<td class="list"><a href="viewMonitoring.jsp?service=<%=name%>">view monitoring data</a></td>
					<td class="list"><a href="shutdownService.jsp?service=<%=name%>">shutdown</a></td>
				</tr><%
			}
		} catch (Throwable t) {
			t.printStackTrace();
			%><pre style="color=red"><%=t.getMessage()%></pre><%
		}
		%>
	</table>
	<form method="post" action="addService.jsp">
		<table border=1px>
			<tr><th>name</th><td><input type="Text" name="name"></td></tr>
			<tr><th>host</th><td><input type="Text" name="host"></td></tr>
			<tr><th>port</th><td><input type="Text" name="port"></td></tr>
			<tr><td colspan="2"><input type="Submit" value="add service"></td></tr>
		</table>
	</form>
<%@include file="../footer.jsp"%>
