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
</head>
<body>
<table height=100% cellspacing=0 cellpadding=0 class=main_table>
<tr>
	<td width=50 background="<%=CPATH%>/img/smsc_02.jpg" rowspan=3></td>
	<td width=30 background="<%=CPATH%>/img/smsc_03.jpg" rowspan=3></td>
	<td bgcolor=#F3F0E3 style="position: relative; top:0; left:0;">
		<!-- background -->
		<table cellspacing=0 cellpadding=0 height=100% style="position: absolute; top:0; left:0; z-index: -1;">
		<tr>
			<td width=12px background="<%=CPATH%>/img/smsc_24.jpg"></td>
			<td>&nbsp;</td>
			<td width=12px background="<%=CPATH%>/img/smsc_26.jpg"></td>
		</tr>
		</table>
		<!-- "SMS Service Center" header -->
		<table cellspacing=0 cellpadding=0>
		<tr>
			<td width=236><IMG SRC="<%=CPATH%>/img/smsc_04.jpg" WIDTH=236 HEIGHT=45 ALT=""></td>
			<td bgcolor="#161E49"><span style="color: #6F77A3; font: bold 11px; vertical-align: -10px;">Preliminary&nbsp;Beta&nbsp;Version</span></td>
			<td width=12 background="<%=CPATH%>/img/smsc_06.jpg"></td>
		</tr>
		</table>
		<%!
		void main_menu_begin_item(JspWriter out, String menuId, String submenuId, String name) throws java.io.IOException
		{
			out.print("<td width=1% style='position:relative; top:0; left:0;'");
			out.print(" onMouseOver='" + submenuId + ".runtimeStyle.visibility=\"visible\";'");
			out.print(" onMouseOut='" + submenuId + ".runtimeStyle.visibility=\"hidden\";'>");
			out.print("<a ID=" + menuId + ">" + name + "</a>");
			out.print("<table cellspacing=0 cellpadding=0 id=" + submenuId + " style=\"position:absolute; top: 20px; left:0px; border: 1px solid #4E6E82; background-color: #3A5867; visibility: hidden;\"");
			out.print(" onMouseOver='" + submenuId + ".runtimeStyle.visibility=\"visible\";'");
			out.print(" onMouseOut='" + submenuId + ".runtimeStyle.visibility=\"hidden\";'>");
		}
		void main_menu_submenu_item(JspWriter out, String url, String id, String name) throws java.io.IOException
		{
			main_menu_submenu_item(out, url, id, name, null, null);
		}
		void main_menu_submenu_item(JspWriter out, String url, String id, String name, String target, String onclick) throws java.io.IOException
		{
			main_menu_submenu_i_fu(out, url + "/index.jsp", id, name, target, onclick);
		}
		void main_menu_submenu_i_fu(JspWriter out, String url, String id, String name) throws java.io.IOException
		{
			main_menu_submenu_i_fu(out, url, id, name, null, null);
		}
		void main_menu_submenu_i_fu(JspWriter out, String url, String id, String name, String target, String onclick) throws java.io.IOException
		{
			out.print("<tr><td><a ID=" + id + " href=\"" +CPATH + url + "\"" + (target != null && target.length() > 0 ? " target=\"" + target + '"' : "") + (onclick != null && onclick.length() > 0 ? " onclick=\"" + onclick + '"' : "") + ">" + name + "</a></td></tr>");
		}
		void main_menu_end_item(JspWriter out) throws java.io.IOException
		{
			out.print("</table></td>");
		}
		%>
		<!-- main menu -->
		<table id=MAIN_MENU_TABLE cellspacing=0 cellpadding=0 class=main_menu>
		<tr>
			<td width=26></td>
			<!--span style="color: #F2E275; font: 11px Verdana;">Home Resources Monitoring Routing</span-->
			<%	
			main_menu_begin_item(out, "MENU0_SMSC_menu", "MENU0_SMSC_submenu", "SMSC");
			main_menu_submenu_item(out, "/smsc_service",               "MENU0_SMSC",             "Configuration");
			main_menu_submenu_item(out, "",                            "MENU0_HOME",             "Status");
			main_menu_submenu_i_fu(out, "/smsc_service/logging.jsp",   "MENU0_SMSC_LOGGING",     "Logging");
			main_menu_submenu_item(out, "/users",                      "MENU0_USERS",            "Users");
			main_menu_submenu_item(out, "/locale_resources",           "MENU0_LOCALE_RESOURCES", "Resources");
			main_menu_end_item(out);
			
			main_menu_begin_item(out, "MENU0_Statistics_menu", "MENU0_Statistics_submenu", "Statistics");
			main_menu_submenu_item(out, "/perfmon", "MENU0_PERFMON", "Monitor", "SMSC Perfomance monitor", "return openPerfMon();");
			main_menu_submenu_item(out, "/smsview", "MENU0_SMSVIEW", "SMS view");
			main_menu_submenu_item(out, "/smsstat", "MENU0_SMSSTAT", "Statistics");
			main_menu_end_item(out);
			
			main_menu_begin_item(out, "MENU0_Routing_menu", "MENU0_Routing_submenu", "Routing");
			main_menu_submenu_item(out, "/aliases",  "MENU0_ALIASES",  "Aliases");
			main_menu_submenu_item(out, "/subjects", "MENU0_SUBJECTS", "Subjects");
			main_menu_submenu_item(out, "/routes",   "MENU0_ROUTES",   "Routes");
			main_menu_submenu_item(out, "/profiles", "MENU0_PROFILES", "Profiles");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Services_menu", "MENU0_Services_submenu", "Services");
			main_menu_submenu_item(out, "/hosts",    "MENU0_HOSTS",    "Hosts");
			main_menu_submenu_item(out, "/services", "MENU0_SERVICES", "Services");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Options_menu", "MENU0_Options_submenu", "Options");
			main_menu_submenu_item(out, "/mscman", "MENU0_MSCMAN", "MSCs");
			main_menu_submenu_item(out, "/dl",     "MENU0_DL",     "Distribution lists");
			main_menu_end_item(out);
			%>
			<td width=100%>&nbsp;</td>
			<td><a ID=MENU0_LOGOUT href="<%=CPATH%>/logout.jsp">Logout</a></td>
			<td width=52></td>
		</tr>
		</table>
		<table id=MAIN_MENU_BACKGROUND_TABLE border=0 cellspacing=0 cellpadding=0 class=main_menu_background>
			<tr>
				<td width=80 background="<%=CPATH%>/img/smsc_13.jpg"></td>
				<td background="<%=CPATH%>/img/smsc_14.jpg">&nbsp;</td>
				<td width=80 background="<%=CPATH%>/img/smsc_16.jpg"></td>
			</tr> 
		</table>
		<script> document.all.<%=MENU0_SELECTION%>.className = 'tmenu_sel'; </script>
<%--table align=center height="100%" width="100%" cellspacing=0 cellpadding=0>
<tr><td width="100%" class=title>SMS Service Center (Preliminary Beta Version)</td></tr>
<tr><td class=tmenu><%
+	%><a ID=MENU0_HOME             href="<%=CPATH%>">Home</a><%
+	%><a ID=MENU0_SMSC             href="<%=CPATH%>/smsc_service/index.jsp">SMSC</a><%
+	%><a ID=MENU0_SMSC_LOGGING     href="<%=CPATH%>/smsc_service/logging.jsp">Logging</a><%
+	%><a ID=MENU0_LOCALE_RESOURCES href="<%=CPATH%>/locale_resources/index.jsp">Resources</a><%
+	%><a ID=MENU0_DL               href="<%=CPATH%>/dl/index.jsp">Distribution lists</a><%
+	%><a ID=MENU0_PERFMON          href="<%=CPATH%>/perfmon/index.jsp" target="SMSC Perfomance monitor" onclick="return openPerfMon();">Monitor</a><%
+	%><a ID=MENU0_SMSVIEW          href="<%=CPATH%>/smsview/index.jsp">SMS view</a><%
+	%><a ID=MENU0_SMSSTAT          href="<%=CPATH%>/smsstat/index.jsp">Statistics</a><%
+	%><a ID=MENU0_MSCMAN           href="<%=CPATH%>/mscman/index.jsp">MSCs</a><%
+	%><a ID=MENU0_USERS            href="<%=CPATH%>/users/index.jsp">Users</a><br><%
+	%><a ID=MENU0_ROUTES           href="<%=CPATH%>/routes/index.jsp">Routes</a><%
+	%><a ID=MENU0_SUBJECTS         href="<%=CPATH%>/subjects/index.jsp">Subjects</a><%
+	%><a ID=MENU0_ALIASES          href="<%=CPATH%>/aliases/index.jsp">Aliases</a><%
+	%><a ID=MENU0_PROFILES         href="<%=CPATH%>/profiles/index.jsp">Profiles</a><%
	%><%
+	%><a ID=MENU0_HOSTS            href="<%=CPATH%>/hosts/index.jsp">Hosts</a><%
+	%><a ID=MENU0_SERVICES         href="<%=CPATH%>/services/index.jsp">Services</a><%
+	%>&nbsp;&nbsp;&nbsp;<a ID=MENU0_LOGOUT href="<%=CPATH%>/logout.jsp">Logout</a><%
	%><script> document.all.<%=MENU0_SELECTION%>.className = 'tmenu_sel'; </script><%
	%></td>
</tr>
<!--Status Line -->
<tr><td class=status><%=STATUS%>&nbsp;</td></tr>
<!-- Status Line END -->
<tr><td width="100%" class=contentTitle><%=TITLE%>&nbsp;</td></tr>
<%@ include file="/WEB-INF/inc/messages_.jsp"--%>