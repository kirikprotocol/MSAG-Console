<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.util.config.*"%>
<%@ page import = "javax.servlet.*"%>
<%@ page import = "javax.servlet.http.*"%>
<%@ page import = "java.util.*"%>
<%
HttpSession ses = request.getSession();
AppContext ctx = (AppContext)request.getAttribute("appContext");
%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
	<title>SMSC Administration application</title>
	<link rel="stylesheet" type="text/css" href="/smsc/smsc/smsc.css">
</head>
<body>
	<table class="header">
		<tr>
			<td><a href="UnderConstruction.html">Config</a></td>
			<td><a href="services/index.jsp">services</a></td>
		</tr>
	</table>