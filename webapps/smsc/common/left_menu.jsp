<ul compact>
	<li><a href="<%=urlPrefix+smscPrefix%>/index.jsp">SMSC</a></li>
	<ul compact>
		<li><a href="<%=urlPrefix+smscPrefix%>/routes.jsp">Routes</a></li>
		<li><a href="<%=urlPrefix+smscPrefix%>/aliases.jsp">Aliases</a></li>
	</ul>
	<li><a href="<%=urlPrefix+hostsPrefix%>/index.jsp">Hosts</a></li>
	<ul compact>
		<%
		Set hosts = serviceManager.getHostNames();
		for (Iterator i = hosts.iterator(); i.hasNext(); )
		{
			String hostName = (String) i.next();
			String encodedHostName = URLEncoder.encode(hostName);
			%><li><a href="<%=urlPrefix+hostsPrefix%>/view_host.jsp?host=<%=encodedHostName%>"><%=hostName%></a></li><%
		} %>
	</ul>
	<li><a href="<%=urlPrefix+servicesPrefix%>/index.jsp">Services</a></li>
	<ul compact>
		<%
		Set services = serviceManager.getServiceNames();
		for (Iterator i = services.iterator(); i.hasNext(); )
		{
			String serviceName = (String) i.next();
			String encodedServiceName = URLEncoder.encode(serviceName);
			%><li><a href="<%=urlPrefix+servicesPrefix%>/view_service.jsp?service=<%=encodedServiceName%>"><%=serviceName%></a></li><%
		} %>
	</ul>
</ul>
