<table class="list" cellspacing="0">
	<TR class="list">
		<TH class="list">Service name</TH>
		<TH class="list">Host name</TH>
		<TH class="list">Status</TH>
		<TH class="list" colSpan=3>Actions</TH>
	</TR>
	<%
	{
		Set hostNames = null;
		if (request.getParameter("host") == null) {
			hostNames = serviceManager.getHostNames();
		} else {
			hostNames = new HashSet(1);
			hostNames.add(request.getParameter("host") );
		}
		for (Iterator i = hostNames.iterator(); i.hasNext(); )
		{
			String hostName = (String) i.next();
			Set serviceNames = serviceManager.getServiceIds(hostName);
			for (Iterator j = serviceNames.iterator(); j.hasNext(); )
			{
				String serviceId = (String) j.next();
				String params = "host=" + URLEncoder.encode(hostName) + "&serviceId=" + URLEncoder.encode(serviceId);
        ServiceInfo info = serviceManager.getServiceInfo(serviceId);
				boolean isRunning = info.getPid() != 0;
        String serviceName = info.getName();
				%>
				<TR class="list">
					<TD class="list"><A href="<%=urlPrefix%>/esme_<%=URLEncoder.encode(serviceId)%>"><%=serviceName%></a></TD>
					<TD class="list"><A href="<%=urlPrefix + hostsPrefix%>/view_host.jsp?<%=params%>"><%=hostName%></a></TD>
					<TD class="list"><%=(isRunning ? "running" : "stopped")%></TD>
					<% if (isRunning) { %>
						<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/shutdown_service.jsp?<%=params%>">shutdown</A></TD>
						<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/edit_service_startup.jsp?<%=params%>">edit startup parameters</A></TD>
						<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/shutdown_and_remove_service.jsp?<%=params%>">shutdown and remove</a></TD>
					<% } else { %>
						<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/start_service.jsp?<%=params%>">start</a></TD>
						<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/edit_service_startup.jsp?<%=params%>">edit startup parameters</a></TD>
						<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/remove_service.jsp?<%=params%>">remove</a></TD>
					<% } %>
				</TR>
				<%
			}
		}
	}
	%>
</TABLE>
