<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html><head>
<title><%=(BROWSER_TITLE==null) ? TITLE : BROWSER_TITLE%></title>
<link rel="STYLESHEET" type="text/css" href="<%=CPATH%>/style.jsp">
</head><body bgcolor="#FFFFFF">
<table align=center width="70%" cellspacing=0 cellpadding=0 class=wnd>
<tr>
	<td width="100%" class=title>SMSC&nbsp;/&nbsp;<%=TITLE%></td>
</tr>
<tr><td class=tmenu><%
	%><a ID=MENU0_HOME href="<%=CPATH%>">status</a><%
	%><a ID=MENU0_ROUTES href="<%=CPATH%>/routes/index.jsp">routes</a><%
	%><a ID=MENU0_SUBJECTS href="<%=CPATH%>/subjects/index.jsp">subjects</a><%
	%><a ID=MENU0_ALIASES href="<%=CPATH%>/aliases/index.jsp">aliases</a><%
	%><a ID=MENU0_PROFILES href="<%=CPATH%>/profiles/index.jsp">profiles</a><%
	%><%
	%><a ID=MENU0_HOSTS href="<%=CPATH%>/hosts/index.jsp">hosts</a><%
	%><a ID=MENU0_SERVICES href="<%=CPATH%>/services/index.jsp">services</a><%
	%><script> document.all.<%=MENU0_SELECTION%>.className = 'tmenu_sel'; </script></td>
</tr>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td class=content>