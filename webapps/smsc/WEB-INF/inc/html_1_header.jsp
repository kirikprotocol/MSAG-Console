<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html><head>
<title><%=(BROWSER_TITLE==null) ? TITLE : BROWSER_TITLE%></title>
<link rel="STYLESHEET" type="text/css" href="<%=CPATH%>/style.jsp">
<%--link rel="STYLESHEET" type="text/css" href="<%=CPATH%>/colors.css">
<link rel="STYLESHEET" type="text/css" href="<%=CPATH%>/common.css"--%>
<script src="<%=CPATH%>/scripts.js" type="text/javascript"></script>
</head><body bgcolor="#FFFFFF">
<table align=center height="100%" width="80%" cellspacing=0 cellpadding=0 class=wnd>
<tr><td width="100%" class=title>SMS Service Center (Preliminary Beta Version)</td></tr>
<tr><td class=tmenu><%
	%><a ID=MENU0_HOME href="<%=CPATH%>">Home</a><%
	%><%--a ID=MENU0_STATUS href="<%=CPATH%>/status/index.jsp">Status</a--%><%
	%><a ID=MENU0_SMS href="<%=CPATH%>/smsc_service/sms/smsview.jsp">SMEs</a><%
	%><a ID=MENU0_ROUTES href="<%=CPATH%>/routes/index.jsp">Routes</a><%
	%><a ID=MENU0_SUBJECTS href="<%=CPATH%>/subjects/index.jsp">Subjects</a><%
	%><a ID=MENU0_ALIASES href="<%=CPATH%>/aliases/index.jsp">Aliases</a><%
	%><a ID=MENU0_PROFILES href="<%=CPATH%>/profiles/index.jsp">Profiles</a><%
	%><%
	%><a ID=MENU0_HOSTS href="<%=CPATH%>/hosts/index.jsp">Hosts</a><%
	%><a ID=MENU0_SERVICES href="<%=CPATH%>/services/index.jsp">Services</a><%
	%>&nbsp;&nbsp;&nbsp;<a ID=MENU0_LOGOUT href="<%=CPATH%>/logout.jsp">Logout</a><%
	%><script> document.all.<%=MENU0_SELECTION%>.className = 'tmenu_sel'; </script><%
	%></td>
</tr>
<!--Status Line -->
<tr><td class=status><%=STATUS%>&nbsp;</td></tr>
<!-- Status Line END -->
<tr><td width="100%" class=contentTitle><%=TITLE%>&nbsp;</td></tr>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<tr><td class=content>