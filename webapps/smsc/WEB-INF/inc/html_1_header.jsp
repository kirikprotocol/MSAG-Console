<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html><head>
<title><%=(BROWSER_TITLE==null) ? TITLE : BROWSER_TITLE%></title><%--
ContextPath:<%=request.getContextPath()%>
PathInfo:<%=request.getPathInfo()%>
PathTranslated:<%=request.getPathTranslated()%>
QueryString:<%=request.getQueryString()%>
RequestURI:<%=request.getRequestURI()%>
RequestURL:<%=request.getRequestURL()%>
Scheme:<%=request.getScheme()%>
ServerName:<%=request.getServerName()%>
ServerPort:<%=request.getServerPort()%>
ServletPath:<%=request.getServletPath()%>  --%>
<base href="<%=request.getScheme()%>://<%=request.getServerName()%>:<%=request.getServerPort() + CPATH%>/">
<%--link rel="STYLESHEET" type="text/css" href="style.jsp"--%>
<link rel="STYLESHEET" type="text/css" href="colors.css">
<link rel="STYLESHEET" type="text/css" href="common.css">
<link rel="STYLESHEET" type="text/css" href="list.css">
<link rel="STYLESHEET" type="text/css" href="properties_list.css">
<link rel="STYLESHEET" type="text/css" href="main_menu.css">
<link rel="STYLESHEET" type="text/css" href="collapsing_tree.css">
<link rel="STYLESHEET" type="text/css" href="navbar.css">
<link rel="STYLESHEET" type="text/css" href="messages.css">
<link rel="STYLESHEET" type="text/css" href="page_menu.css">
<script src="scripts.js" type="text/javascript"></script>
<script>
function openPerfMon()
{
	open("perfmon/index.jsp", null, "channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=420,width=480");
	return false;
}
</script>
</head>
<body>
<%
if (isServiceStatusNeeded || (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0))
{%><%@ include file="/WEB-INF/inc/service_status.jsp"%><%}
%>
<table height=100% cellspacing=0 cellpadding=0 class=main_table>
<tr>
	<td width=50 background="img/smsc_02.jpg" rowspan=3></td>
	<td width=30 background="img/smsc_03.jpg" rowspan=3></td>
	<td bgcolor=#F3F0E3 style="position: relative; top:0; left:0;">
		<!-- background -->
		<table cellspacing=0 cellpadding=0 height=100% style="position: absolute; top:0; left:0; z-index: -1;">
		<tr>
			<td width=12px background="img/smsc_24.jpg"></td>
			<td>&nbsp;</td>
			<td width=12px background="img/smsc_26.jpg"></td>
		</tr>
		</table>
		<!-- "SMS Service Center" header -->
		<table cellspacing=0 cellpadding=0>
		<tr>
			<td width=236><IMG SRC="img/smsc_04.jpg" WIDTH=236 HEIGHT=45 ALT=""></td>
			<td bgcolor="#161E49"><span style="color: #6F77A3; font: bold 11px; vertical-align: -10px;">Preliminary&nbsp;Beta&nbsp;Version</span></td>
			<td width=12 background="img/smsc_06.jpg"></td>
		</tr>
		</table>
		<%!
		void main_menu_begin_item(JspWriter out, String menuId, String submenuId, String name) throws java.io.IOException
		{
			out.print("<td width=1% style='position:relative; top:0; left:0;'");
			out.print(" onMouseOver='" + submenuId + ".runtimeStyle.visibility=\"visible\";'");
			out.print(" onMouseOut='" + submenuId + ".runtimeStyle.visibility=\"hidden\";'>");
			out.print("<a ID=" + menuId + ">" + name + "</a>");
			out.print("<table class=submenu cellspacing=0 cellpadding=0 id=" + submenuId);
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
			out.print("<tr><td ID=" + id + " nowrap><a href=\"" + CPATH + url + "\"" + (target != null && target.length() > 0 ? " target=\"" + target + '"' : "") + (onclick != null && onclick.length() > 0 ? " onclick=\"" + onclick + '"' : "") + ">" + name + "</a></td></tr>");
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
			<%
			main_menu_begin_item(out, "MENU0_SMSC_menu", "MENU0_SMSC_submenu", "SMSC");
			main_menu_submenu_item(out, "/smsc_service",               "MENU0_SMSC",             "Configuration");
			main_menu_submenu_i_fu(out, "/smsc_service/status.jsp",    "MENU0_SMSC_Status",      "Status");
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
			<td><a ID=MENU0_LOGOUT href="logout.jsp">Logout</a></td>
			<td width=52></td>
		</tr>
		</table>
		<table id=MAIN_MENU_BACKGROUND_TABLE border=0 cellspacing=0 cellpadding=0 class=main_menu_background>
			<tr>
				<td width=80 background="img/smsc_13.jpg"></td>
				<td background="img/smsc_14.jpg">&nbsp;</td>
				<td width=80 background="img/smsc_16.jpg"></td>
			</tr>
		</table>
		<%-- SMSC Status --%>
		<table cellpadding=0 cellspacing=0 height=30px class=smsc_status>
		<tr>
			<th background="img/smsc_17.jpg" nowrap><%=TITLE%></th>
			<td >&nbsp;<%if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0){
				%><%=StringEncoderDecoder.encode(ServiceIDForShowStatus)%>&nbsp;is&nbsp;<%=serviceStatus(ServiceIDForShowStatus)%><%
			}%></td>
			<td width=12px background="img/smsc_19.jpg"></td>
		</tr>
		</table>
		<span id="MENU0_NONE"></span>
		<script> document.all.<%=MENU0_SELECTION%>.className = 'submenu_sel'; </script>
<%--Status Line -->
<tr><td class=status><%=STATUS%>&nbsp;</td></tr>
<!-- Status Line END -->
<tr><td width="100%" class=contentTitle><%=TITLE%>&nbsp;</td></tr--%>
<%@ include file="/WEB-INF/inc/messages.jsp"%>