<%@include file="header.jsp"%>
<%! static final protected String paramHeader = "SMSC_METHOD_PARAMETER_";%>
<%
	String host = request.getParameter("host");
	String service = request.getParameter("service");
	if (host == null || host.equals(""))
		throw new Exception("Host not specified");
	if (service == null || service.equals(""))
		throw new Exception("Service not specified");
%>
	<%=link("<h1>Host: \"" + host +"\"</h1>", "viewHost.jsp", "host", host)%>
	<h2>Service: "<%=service%>:</h2>
	<%
	ServiceInfo info = serviceManager.getServiceInfo(service);
	if (info.getPid() != 0)
	{
	%>
		<table class="list" cellspacing="0">
			<tr class="list">
				<th class="list">name</th>
				<th class="list">method</th>
				<th class="list">method return type</th>
				<th class="list">arguments</th>
			</tr>
			<%
			try {
				Map comps = info.getComponents();
				for (Iterator i = comps.values().iterator(); i.hasNext(); )
				{
					Component c = (Component) i.next();
					Map meths = c.getMethods();
					%>
					<tr class="list">
					</tr>
					<%
					boolean isComponentNameAlreadyDisplayed = false;
					for (Iterator j = meths.values().iterator(); j.hasNext(); ) 
					{
						Method m = (Method)j.next();
						%>
						<tr class="list">
							<% if (!isComponentNameAlreadyDisplayed) { %>
		 						<td class="list" rowspan="<%=meths.size()%>"><%=c.getName()%></td>
								<%
								isComponentNameAlreadyDisplayed = true;
							}%>
							<td class="list"><%=m.getName()%></td>
							<td class="list"><%=m.getType().getName()%></td>
							<td class="list">
								<form action="callMethod.jsp" method="post">
								<input type="Hidden" name="host" value="<%=host%>">
								<input type="Hidden" name="service" value="<%=service%>">
								<input type="Hidden" name="component" value="<%=c.getName()%>">
								<input type="Hidden" name="method" value="<%=m.getName()%>">
								<input type="Hidden" name="returnTypeName" value="<%=m.getType().getName()%>">
								<table>
									<tr>
										<%
										for (Iterator k = m.getParams().values().iterator(); k.hasNext(); )
										{
											Parameter p = (Parameter) k.next();
											%>
											<td class="list"><%=p.getName()%>:<input type="Text" name="<%=paramHeader+p.getName()%>"></td>
											<%
										}
										%>
										<td><input type="Submit" value="call"></td>
									</tr>
								</table>
								</form>
							</td>
						</tr>
						<%
					}
				}
			} catch (Throwable t) {
				t.printStackTrace();
				%><pre style="color=red"><%=t.getMessage()%></pre><%
			}
			%>
		</table>
	<%
	} else {
	%>
		<h3 align="center">Service not started</h3>
	<%
	}
	%>
<%@include file="footer.jsp"%>
