<%{%>
<ul compact>
	<li><a href="<%=urlPrefix+smscPrefix%>/index.jsp">SMSC</a></li>
	<ul compact>
		<li><a href="<%=urlPrefix+routesPrefix%>/index.jsp">Routes</a></li>
		<li><a href="<%=urlPrefix+subjectsPrefix%>/index.jsp">Subjects</a></li>
		<li><a href="<%=urlPrefix+aliasesPrefix%>/index.jsp">Aliases</a></li>
		<li><a href="<%=urlPrefix+profilesPrefix%>/index.jsp">Profiles</a></li>
	</ul>
	<li><a href="<%=urlPrefix+hostsPrefix%>/index.jsp">Hosts</a></li>
	<ul compact>
		<%
		if (serviceManager != null)
		{
			Set hosts = serviceManager.getHostNames();
			for (Iterator i = hosts.iterator(); i.hasNext(); )
			{
				String hostName = (String) i.next();
				%><li><a href="<%=urlPrefix+hostsPrefix%>/view_host.jsp?host=<%=URLEncoder.encode(hostName)%>"><%=StringEncoderDecoder.encode(hostName)%></a></li><%
			}
		}
		%>
	</ul>
	<li><a href="<%=urlPrefix+servicesPrefix%>/index.jsp">Services</a></li>
	<ul compact>
		<%
		if (serviceManager != null)
		{
			List smes = serviceManager.getSmeIds();
			for (Iterator i = smes.iterator(); i.hasNext(); )
			{
				String smeId = (String) i.next();
				String encodedSmeId = URLEncoder.encode(smeId);
				if (serviceManager.isService(smeId))
				{
					%><li><a href="<%=urlPrefix%>/esme_<%=URLEncoder.encode(smeId)%>"><%=StringEncoderDecoder.encode(smeId)%></a></li><%
				}
				else
				{
					%><li><%=StringEncoderDecoder.encode(smeId)%></li><%
				}
			}
		}%>
	</ul>
</ul>
<%}%>
