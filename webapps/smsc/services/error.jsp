<%@ page isErrorPage="true" %>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="../header.jsp"%>
<% String message = request.getParameter("message");%>
<h1 align="center" style="color=red"><%=message == null || message.equals("") ? "ERROR occured" : "ERROR occured:<br>"+message%></h1>
<% if (exception != null) {
	%><h2>Exception: <%=exception.getMessage()%></h2><%=exception.toString()%><%
}
%>
<%@include file="../footer.jsp"%>