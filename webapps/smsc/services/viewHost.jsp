<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="header.jsp"%>
<%
	String host = request.getParameter("host");
	if (host == null || host.equals(""))
		throw new Exception("Host not specified");
%>
	<h1>Host "<%=host%>":</h1>
	<table class="list" cellspacing="0">
		<tr class="list">
			<th class="list">host</th>
			<th class="list">name</th>
			<th class="list">port</th>
			<th class="list">command line</th>
			<th class="list">arguments</th>
			<th class="list">PID</th>
			<th  class="list" colspan=4>actions</th>
		</tr>
		<%
		try {
			Set names = serviceManager.getServiceNames(host);
			for (Iterator i = names.iterator(); i.hasNext(); ) {
				ServiceInfo s = serviceManager.getServiceInfo((String)i.next());
				%><tr class="list">
					<td class="list"><%=s.getHost()%></td>
					<td class="list"><%=s.getName()%></td>
					<td class="list"><%=s.getPort()%></td>
					<td class="list"><%=s.getCmdLine()%></td>
					<td class="list"><%=s.getArgs().toString()%></td>
					<td class="list"><%=s.getPid() == 0 ? "stopped" : String.valueOf(s.getPid())%></td>
					<td class="list"><%=action("start", "startService.jsp", "host", s.getHost(), "service", s.getName())%></td>
					<td class="list"><%=action("shutdown", "shutdownService.jsp", "host", s.getHost(), "service", s.getName())%></td>
					<td class="list"><%=action("kill", "killService.jsp", "host", s.getHost(), "service", s.getName())%></td>
					<td class="list"><%=action("remove", "removeService.jsp", "host", s.getHost(), "service", s.getName())%></td>
				</tr><%
			}
		} catch (Throwable t) {
			t.printStackTrace();
			%><pre style="color=red"><%=t.getMessage()%></pre><%
		}
		%>
	</table>
	<form method="post" action="addService.jsp">
		<input type="Hidden" name="host" value="<%=host%>">
		<table border=1px>
			<tr><th>name</th><td><input type="Text" name="service" value="dumb"></td></tr>
			<tr><th>port</th><td><input type="Text" name="port" value="6677"></td></tr>
			<tr><th>command line</th><td><input type="Text" name="cmdLine" value="/export/home/igork/build/bin/admin/service/smsc_dumb_service"></td></tr>
			<tr><td colspan="2"><input type="Submit" value="add service"></td></tr>
		</table>
	</form>
<%@include file="footer.jsp"%>
