<%@ page isErrorPage="true" %>
<%@ page import = "java.io.PrintWriter"%>
<%@include file="/common/header.jsp"%>
<% String message = request.getParameter("message");%>
<h1 align="center" style="color=red"><%=message == null || message.equals("") ? "ERROR occured" : "ERROR occured:<br>"+message%></h1>
<h3>Request:</h3>
<dl>
	<dt>Params:</dt>
		<dd>
			<table class="data"><%
				for (Iterator i=request.getParameterMap().keySet().iterator(); i.hasNext(); )
				{
					String s = (String)i.next();
					String value = request.getParameter(s);
					%><tr><th><%=s%></th><td><%=value%></td></tr><%
				}%>
			</table>
		</dd>
	<dt>Context path:</dt>
		<dd><%=request.getContextPath()%></dd>
	<dt>Method:</dt>
		<dd><%=request.getMethod()%></dd>
	<dt>Request URI:</dt>
		<dd><%=request.getRequestURI()%></dd>
</dl>
<% 
if (exception != null) {
	%><h2 style="color: Red;">Exception: <%=exception.getMessage()%></h2><%=exception.toString()%>
	<pre><% exception.printStackTrace(new PrintWriter(out));%></pre><%
}

try {
	serviceManager.refreshServices();
} catch (Throwable e)
{
}

%>
<%@include file="footer.jsp"%>