<%@ page errorPage="error.jsp" %>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="../header.jsp"%>
<% 
	ServiceManager manager = ctx.getServiceManager();
	Config conf = manager.getServiceConfig(request.getParameter("service"));
%>
<form method="post" action="setServiceConfig.jsp">
	<table>
		<% for (Iterator i=conf.getParameterNames().iterator(); i.hasNext(); )
		{
			String name = (String)i.next();
			%>
			<tr>
				<td><%=name%></td>
				<td><input type="Text" name="<%=name%>" value="<%=conf.getParameter(name)%>"></td>
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
<a href="index.jsp">Services</a>
<%@include file="../footer.jsp"%>
