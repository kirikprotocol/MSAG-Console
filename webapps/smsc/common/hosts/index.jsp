<%@ include file="/common/header.jsp"%>
<h2>Hosts:</h2>
<TABLE class="list" cellspacing="0">
	<TR class="list">
		<TH class="list">Host name</TH>
		<th class="list" width="10%">all services</th>
		<th class="list" width="10%">running services</th>
		<TH class="list" colspan="2">&nbsp;</TH>
	</TR>
	
		<%
		try {
			Set names = serviceManager.getHostNames();
			for (Iterator i = names.iterator(); i.hasNext(); ) {
				String name = (String)i.next();
				String encodedName = URLEncoder.encode(name);
				int allServices = serviceManager.getCountServices(name);
				int runningServices = serviceManager.getCountRunningServices(name);
				%>
				<tr class="list">
					<td class="list"><a href="view_host.jsp?host=<%=encodedName%>"><%=encodedName%></a></td>
					<td class="list"><%=String.valueOf(allServices)%></td>
					<td class="list"><%=String.valueOf(runningServices)%></td>
					<td class="list"><a href="edit_host.jsp?host=<%=encodedName%>">Edit host startup parameters</a></td>
					<td class="list"><a href="<%=(runningServices == 0 ? "remove_host.jsp" : "remove_host_with_services.jsp")+"?host="+encodedName%>">remove</a></td>
				</tr><%
			}
		} catch (Throwable t) {
			t.printStackTrace();
			%><tr><td colspan="5"><pre style="color=red"><%=t.toString()%></pre></td></tr><%
		}
		%>
</TABLE>
<a href="add_host.jsp">Add host</a>
<%@ include file="/common/footer.jsp"%>
