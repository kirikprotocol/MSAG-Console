<%@ page errorPage="error.jsp" %>
<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.util.config.*"%>
<%@ page import = "javax.servlet.*"%>
<%@ page import = "javax.servlet.http.*"%>
<%@ page import = "java.util.*"%>
<%@ page import = "java.net.URLEncoder"%>
<%! 
	String actionButton(String name, String url, String[][] params)
	{
		String result = "<form class=\"action\" action=\"" + URLEncoder.encode(url) + "\" method=post>";
		if (params != null)
			for (int i=0; i<params.length; i++)
        result += "<input type=hidden name=\"" + URLEncoder.encode(params[i][0]) + "\" value=\"" + URLEncoder.encode(params[i][1]) + "\">";
     result += "<input class=\"action\" type=submit value=\"" + name + "\"></form>";
		return result;
	}
	String actionButton(String name, String url)
	{
		return actionButton(name, url, new String[0][0]);
	}
	String actionButton(String name, String url, String pName1, String p1)
	{
		String[][] params = {{pName1, p1}};
		return actionButton(name, url, params);
	}
	String actionButton(String name, String url, String pName1, String p1, String pName2, String p2)
	{
		String[][] params = {{pName1, p1}, {pName2, p2}};
		return actionButton(name, url, params);
	}

	String link(String name, String url)
	{
		return "<a class=link href=\"" + URLEncoder.encode(url) + "\">" + URLEncoder.encode(name) + "</a>";
	}
	String link(String name, String url, String pName1, String p1)
	{
		return "<a class=link href=\"" + URLEncoder.encode(url) 
			+ '?' + URLEncoder.encode(pName1) + '=' + URLEncoder.encode(p1) 
			+ "\">" + name + "</a>";
	}
	String link(String name, String url, String pName1, String p1, String pName2, String p2)
	{
		return "<a class=link href=\"" + URLEncoder.encode(url) 
			+ '?' + URLEncoder.encode(pName1) + '=' + URLEncoder.encode(p1) 
			+ '&' + URLEncoder.encode(pName2) + '=' + URLEncoder.encode(p2) 
			+ "\">" + name + "</a>";
	}

	String action(String name, String url)
   {
		return link(name, url);
	}
	String action(String name, String url, String pName1, String p1)
   {
		return link(name, url, pName1, p1);
	}
	String action(String name, String url, String pName1, String p1, String pName2, String p2)
   {
		return link(name, url, pName1, p1, pName2, p2);
	}

%>
<%
HttpSession ses = request.getSession();
AppContext appContext = (AppContext)request.getAttribute("appContext");
ServiceManager serviceManager = appContext.getServiceManager();
%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
	<title>SMSC Administration application</title>
	<link rel="stylesheet" type="text/css" href="../smsc.css">
</head>
<body>
	<table class="header">
		<tr>
			<td><%=action("Config", "../UnderConstruction.html")%></td>
			<td><%=action("services",  "index.jsp")%></td>
		</tr>
	</table>