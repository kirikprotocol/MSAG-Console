<%
{
	Set hostNames = null;
	boolean isHostColumnNeeded = request.getParameter("host") == null;
	if (isHostColumnNeeded) {
		hostNames = serviceManager.getHostNames();
	} else {
		hostNames = new HashSet(1);
		hostNames.add(request.getParameter("host"));
	}%>
	<table class="list" cellspacing="0">
		<TR class="list">
			<TH class="list">Service&nbsp;ID</TH>
			<%if (isHostColumnNeeded) {%>
				<TH class="list">Host&nbsp;name</TH>
			<%}%>
			<TH class="list">Status</TH>
			<TH class="list" colSpan=3>Actions</TH>
		</TR>
		<%
		for (Iterator i = hostNames.iterator(); i.hasNext(); )
		{
			String hostName = (String) i.next();
			Set serviceNames = serviceManager.getServiceIds(hostName);
			for (Iterator j = serviceNames.iterator(); j.hasNext(); )
			{
				String _serviceId = (String) j.next();
				String params = "host=" + URLEncoder.encode(hostName) + "&serviceId=" + URLEncoder.encode(_serviceId);
				ServiceInfo info = serviceManager.getServiceInfo(_serviceId);
				boolean isRunning = info.getPid() != 0;
				//String serviceName = info.getName();
				%>
				<TR class="list">
				<TD class="list"><A href="<%=urlPrefix%>/esme_<%=URLEncoder.encode(_serviceId)%>/"><%=StringEncoderDecoder.encode(_serviceId)%></a></TD>
				<%if (isHostColumnNeeded) {%>
					<TD class="list"><A href="<%=urlPrefix + hostsPrefix%>/view_host.jsp?<%=params%>"><%=StringEncoderDecoder.encode(hostName)%></a></TD>
				<%}%>
				<TD class="list"><%=(isRunning ? "running" : "stopped")%></TD>
				<%if (isRunning) {%>
					<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/shutdown_service.jsp?<%=params%>">shutdown</A></TD>
					<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/edit_service_startup.jsp?<%=params%>">edit startup parameters</A></TD>
					<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/shutdown_and_remove_service.jsp?<%=params%>">shutdown and remove</a></TD>
				<%} else {%>
					<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/start_service.jsp?<%=params%>">start</a></TD>
					<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/edit_service_startup.jsp?<%=params%>">edit startup parameters</a></TD>
					<TD class="list"><A href="<%=urlPrefix + servicesPrefix%>/remove_service.jsp?<%=params%>">remove</a></TD>
				<%}%>
				</TR>
				<%
			}
		}
		// show SMEs that is not Services
		for (Iterator i = serviceManager.getSmeIds().iterator(); i.hasNext(); )
		{
			String smeId = (String) i.next();
			String params = "serviceId=" + URLEncoder.encode(smeId);
			boolean isRunning = true;
			%>
			<TR class="list">
			<%if (isHostColumnNeeded) {%>
				<TD class="list" colspan="2"><%=StringEncoderDecoder.encode(smeId)%></TD>
			<%} else {%>
				<TD class="list"><%=StringEncoderDecoder.encode(smeId)%></TD>
			<%}%>
			<TD class="list"><%=(isRunning ? "running" : "stopped")%></TD>
			<%if (isRunning) {%>
				<TD class="list" colspan="3">&nbsp;</TD>
			<%} else {%>
				<TD class="list" colspan="3"><A href="<%=urlPrefix + servicesPrefix%>/remove_service.jsp?<%=params%>">remove</a></TD>
			<%}%>
			</TR>
			<%
		}
		%>
	</TABLE>
<%}%>
