<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "javax.servlet.*"%>
<%@ page import = "javax.servlet.http.*"%>
<%
HttpSession ses = request.getSession();
AppContext ctx = (AppContext)request.getAttribute("appContext");
%>
<HTML>
<BODY>
Context config obj: <%=ses%>
</BODY>
</HTML>
