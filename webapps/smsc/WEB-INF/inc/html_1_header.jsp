<%@
 include file="/WEB-INF/inc/page_menu.jsp"%><%@
 include file="/WEB-INF/inc/buttons.jsp"
%><table height="100%" cellspacing=0 cellpadding=0 class=main_table>
<tr>
	<td width=50 background="/images/smsc_02.jpg" rowspan=3></td>
	<td width=30 background="/images/smsc_03.jpg" rowspan=3></td>
	<td bgcolor="#F3F0E3" style="position: relative; top:0; left:0;">
		<!-- background -->
		<table cellspacing=0 cellpadding=0 height="100%" style="position: absolute; top:0; left:0; z-index: -1;">
		<tr>
			<td width=12px background="/images/smsc_24.jpg"></td>
			<td>&nbsp;</td>
			<td width=12px background="/images/smsc_26.jpg"></td>
		</tr>
		</table>
		<!-- "SMS Service Center" header -->
		<table cellspacing=0 cellpadding=0>
		<tr>
			<td width=236><IMG SRC="/images/smsc_04.jpg" WIDTH=236 HEIGHT=45 ALT=""></td>
			<td bgcolor="#161E49"><span style="color: #6F77A3; font: bold 11px; vertical-align: -10px;">Version&nbsp;1.0</span></td>
			<td width=12 background="/images/smsc_06.jpg"></td>
		</tr>
		</table>
		<%!
		void main_menu_begin_item(JspWriter out, String menuId, String submenuId, String name) throws java.io.IOException
		{
			out.print("<td width=1% style='position:relative; top:0; left:0;'");
      out.print(" onMouseOver='" + submenuId + ".runtimeStyle.visibility=\"visible\";document.all." + submenuId + "_h.runtimeStyle.visibility=\"visible\";");
      out.print("document.all." + submenuId + "_h.runtimeStyle.width="+submenuId+".offsetWidth;");
      out.print("document.all." + submenuId + "_h.runtimeStyle.height="+submenuId+".offsetHeight;");
      out.print("' onMouseOut='" + submenuId + ".runtimeStyle.visibility=\"hidden\";document.all." + submenuId + "_h.runtimeStyle.visibility=\"hidden\";'>");
			out.print("<a ID=" + menuId + ">" + name + "</a>");
      out.print("<iframe id=" + submenuId + "_h class=hiddenLayer src='/images/blank.html' width='100%' height='0px'></iframe>");
			out.print("<table class=submenu cellspacing=0 cellpadding=0 id=" + submenuId);
			out.print(" onMouseOver='" + submenuId + ".runtimeStyle.visibility=\"visible\";document.all." + submenuId + "_h.runtimeStyle.visibility=\"visible\";");
      out.print("document.all." + submenuId + "_h.runtimeStyle.width="+submenuId+".offsetWidth;");
      out.print("document.all." + submenuId + "_h.runtimeStyle.height="+submenuId+".offsetHeight;");
			out.print("' onMouseOut='" + submenuId + ".runtimeStyle.visibility=\"hidden\";document.all." + submenuId + "_h.runtimeStyle.visibility=\"hidden\";'>");
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
			out.print("</table>");
      out.print("</td>");
		}
		%>
		<!-- main menu -->
		<table id=MAIN_MENU_TABLE cellspacing=0 cellpadding=0 class=main_menu>
		<tr>
			<td width=26></td>
			<%
			main_menu_begin_item(out, "MENU0_SMSC_menu", "MENU0_SMSC_submenu", "SMSC");
			main_menu_submenu_item(out, "/smsc_service",               "MENU0_SMSC",             "Configuration");
      main_menu_submenu_item(out, "/reshedule",                  "MENU0_SMSC_Reshedule",   "Reschedule");
      main_menu_submenu_item(out, "/directives",                 "MENU0_SMSC_Directives",  "Directives");
			main_menu_submenu_i_fu(out, "/smsc_service/status.jsp",    "MENU0_SMSC_Status",      "Status");
			main_menu_submenu_i_fu(out, "/smsc_service/logging.jsp",   "MENU0_SMSC_LOGGING",     "Logging");
			main_menu_submenu_item(out, "/users",                      "MENU0_USERS",            "Users");
			main_menu_submenu_item(out, "/locale_resources",           "MENU0_LOCALE_RESOURCES", "Resources");
      main_menu_submenu_item(out, "/journal",                    "MENU0_JOURNAL",          "Journal");
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
			main_menu_submenu_item(out, "/tracer",   "MENU0_TRACER",   "Routes tracing");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Profiles_menu", "MENU0_Profiles_submenu", "Profiles");
			main_menu_submenu_item(out, "/profiles",            "MENU0_PROFILES",        "List");
			main_menu_submenu_i_fu(out, "/profiles/groups.jsp", "MENU0_PROFILES_GROUPS", "Groups");
			main_menu_submenu_i_fu(out, "/profiles/lookup.jsp", "MENU0_PROFILES_LOOKUP", "Lookup profile");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Services_menu", "MENU0_Services_submenu", "Services");
			main_menu_submenu_item(out, "/hosts",    "MENU0_HOSTS",    "Hosts");
			main_menu_submenu_item(out, "/services", "MENU0_SERVICES", "Services");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Options_menu", "MENU0_Options_submenu", "Options");
			main_menu_submenu_item(out, "/mscman",            "MENU0_MSCMAN",        "MSCs");
			main_menu_submenu_item(out, "/dl",                "MENU0_DL",            "Distribution lists");
      main_menu_submenu_i_fu(out, "/dl/principals.jsp", "MENU0_DL_PRINCIPALS", "Distribution lists - principals");
			main_menu_end_item(out);
			%>
			<td width="100%">&nbsp;</td>
			<td><a ID=MENU0_LOGOUT href="logout.jsp">Logout</a></td>
			<td width=52></td>
		</tr>
		</table>
		<table id=MAIN_MENU_BACKGROUND_TABLE border=0 cellspacing=0 cellpadding=0 class=main_menu_background>
			<tr>
				<td width=80 background="/images/smsc_13.jpg"></td>
				<td background="/images/smsc_14.jpg">&nbsp;</td>
				<td width=80 background="/images/smsc_16.jpg"></td>
			</tr>
		</table>
		<span id="MENU0_NONE"></span>
		<script> document.all.<%=MENU0_SELECTION%>.className = 'submenu_sel'; </script>