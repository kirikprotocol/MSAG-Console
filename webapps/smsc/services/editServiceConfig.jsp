<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="header.jsp"%>
<% 
	String name = request.getParameter("service");
	String host = request.getParameter("host");
	Config conf = serviceManager.getServiceConfig(name);
%>
<form method="post" action="setServiceConfig.jsp">
	<input type="Hidden" name="service" value="<%=URLEncoder.encode(name)%>">
	<input type="Hidden" name="host"    value="<%=URLEncoder.encode(host)%>">
	<table>
		<% for (Iterator i=conf.getParameterNames().iterator(); i.hasNext(); )
		{
			String paramName = (String)i.next();
			%>
			<tr>
				<td><%=paramName%></td>
				<td><input type="Text" name="<%=paramName%>" value="<%=conf.getParameter(paramName)%>"></td>
			</tr>
			<%
		}
		%>
		<tr>
			<td><input type="Submit" name="submit" value="Apply"></td>
			<td><input type="Reset"></td>
		</tr>
	</table>
</form>
<%=action("Host \""+host+'"', "viewHost.jsp", "host", host)%>
<%@include file="footer.jsp"%>
