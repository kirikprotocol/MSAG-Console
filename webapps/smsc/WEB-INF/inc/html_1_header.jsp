<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html><head>
<title><%=(BROWSER_TITLE==null) ? TITLE : BROWSER_TITLE%></title>
<%--link rel="STYLESHEET" type="text/css" href="<%=CPATH%>/style.jsp"--%>
<link rel="STYLESHEET" type="text/css" href="<%=CPATH%>/colors.css">
<link rel="STYLESHEET" type="text/css" href="<%=CPATH%>/common.css">
<script src="<%=CPATH%>/scripts.js" type="text/javascript"></script>
<script>
function openPerfMon()
{
	open("<%=CPATH%>/perfmon/index.jsp", null, "channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=420,width=480");
	return false;
}
</script>
</head><body bgcolor="#FFFFFF">
<table align=center height="100%" width="80%" cellspacing=0 cellpadding=0 class=wnd>
<tr><td width="100%" class=title>SMS Service Center (Preliminary Beta Version)</td></tr>
<tr><td class=tmenu><%
	%><a ID=MENU0_HOME             href="<%=CPATH%>">Home</a><%
	%><a ID=MENU0_SMSC             href="<%=CPATH%>/smsc_service/index.jsp">SMSC</a><%
	%><a ID=MENU0_SMSC_LOGGING     href="<%=CPATH%>/smsc_service/logging.jsp">Logging</a><%
	%><a ID=MENU0_LOCALE_RESOURCES href="<%=CPATH%>/locale_resources/index.jsp">Resources</a><%
	%><a ID=MENU0_DL               href="<%=CPATH%>/dl/index.jsp">Distribution lists</a><%
	%><a ID=MENU0_PERFMON          href="<%=CPATH%>/perfmon/index.jsp" target="SMSC Perfomance monitor" onclick="return openPerfMon();">Monitor</a><%
	%><a ID=MENU0_SMSVIEW          href="<%=CPATH%>/smsview/index.jsp">SMS view</a><%
	%><a ID=MENU0_SMSSTAT          href="<%=CPATH%>/smsstat/index.jsp">Statistics</a><%
	%><a ID=MENU0_MSCMAN           href="<%=CPATH%>/mscman/index.jsp">MSCs</a><%
	%><a ID=MENU0_USERS            href="<%=CPATH%>/users/index.jsp">Users</a><br><%
	%><a ID=MENU0_ROUTES           href="<%=CPATH%>/routes/index.jsp">Routes</a><%
	%><a ID=MENU0_SUBJECTS         href="<%=CPATH%>/subjects/index.jsp">Subjects</a><%
	%><a ID=MENU0_ALIASES          href="<%=CPATH%>/aliases/index.jsp">Aliases</a><%
	%><a ID=MENU0_PROFILES         href="<%=CPATH%>/profiles/index.jsp">Profiles</a><%
	%><%
	%><a ID=MENU0_HOSTS            href="<%=CPATH%>/hosts/index.jsp">Hosts</a><%
	%><a ID=MENU0_SERVICES         href="<%=CPATH%>/services/index.jsp">Services</a><%
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
