<%{%>
<ul compact>
	<li><a href="<%=urlPrefix+smscPrefix%>/index.jsp">SMSC</a></li>
	<ul compact>
		<li><a href="<%=urlPrefix+routesPrefix%>/index.jsp">Routes</a></li>
		<li><a href="<%=urlPrefix+subjectsPrefix%>/index.jsp">Subjects</a></li>
		<li><a href="<%=urlPrefix+aliasesPrefix%>/index.jsp">Aliases</a></li>
	</ul>
	<li><a href="<%=urlPrefix+hostsPrefix%>/index.jsp">Hosts</a></li>
	<ul compact>
		<%
		Set hosts = serviceManager.getHostNames();
		for (Iterator i = hosts.iterator(); i.hasNext(); )
		{
			String hostName = (String) i.next();
			%><li><a href="<%=urlPrefix+hostsPrefix%>/view_host.jsp?host=<%=URLEncoder.encode(hostName)%>"><%=StringEncoderDecoder.encode(hostName)%></a></li><%
		} %>
	</ul>
	<li><a href="<%=urlPrefix+servicesPrefix%>/index.jsp">Services</a></li>
	<ul compact>
		<%
		Set services = serviceManager.getServiceIds();
		for (Iterator i = services.iterator(); i.hasNext(); )
		{
			String _serviceId = (String) i.next();
			String encodedServiceId = URLEncoder.encode(_serviceId);
			%><li><a href="<%=urlPrefix%>/esme_<%=URLEncoder.encode(_serviceId)%>"><%=StringEncoderDecoder.encode(_serviceId)%></a></li><%
		} %>
	</ul>
</ul>
<%}%>
