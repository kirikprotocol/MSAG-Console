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
		Set services = serviceManager.getServiceIds();
		for (Iterator i = services.iterator(); i.hasNext(); )
		{
			String serviceId = (String) i.next();
			String serviceName = serviceManager.getServiceInfo(serviceId).getName();
			String encodedServiceId = URLEncoder.encode(serviceId);
			%><li><a href="<%=urlPrefix%>/esme_<%=URLEncoder.encode(serviceId)%>"><%=serviceName%></a></li><%
		} %>
	</ul>
</ul>
