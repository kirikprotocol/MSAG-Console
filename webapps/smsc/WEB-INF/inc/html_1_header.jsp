<%@ include file="/WEB-INF/inc/page_menu.jsp"%>
<%@ include file="/WEB-INF/inc/buttons.jsp"%>

<table height="100%" cellspacing="0" cellpadding="0" class="main_table">
<tr>
	<td width="50" background="/images/smsc_02.jpg" rowspan="3"></td>
	<td width="30" background="/images/smsc_03.jpg" rowspan="3"></td>
	<td bgcolor="#F3F0E3" style="position: relative; top:0; left:0;">
		<!-- background -->
		<table cellspacing="0" cellpadding="0" height="100%" style="position: absolute; top:0; left:0; z-index: -1;">
        <tr>
			<td width="12px" background="/images/smsc_24.jpg"></td>
			<td>&nbsp;</td>
			<td width="12px" background="/images/smsc_26.jpg"></td>
		</tr>
		</table>
		<!-- "SMS Service Center" header -->
		<table cellspacing="0" cellpadding="0" border="0" >
		<tr>
			<td width="236"><IMG SRC="/images/smsc_04.jpg" WIDTH="236" HEIGHT="45" ALT=""></td>
			<td bgcolor="#161E49"><span style="color: #6F77A3; font: bold 11px; vertical-align: -10px;"><%=getLocString("common.version")%></span></td>
			<td width="12" background="/images/smsc_06.jpg"></td>
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
			out.print("<a ID=" + menuId + ">" + getLocString(name) + "</a>");
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
			out.print("<tr><td ID=" + id + " nowrap><a href=\"" + CPATH + url + "\"" + (target != null && target.length() > 0 ? " target=\"" + target + '"' : "") + (onclick != null && onclick.length() > 0 ? " onclick=\"" + onclick + '"' : "") + ">" + getLocString(name) + "</a></td></tr>");
		}
		void main_menu_end_item(JspWriter out) throws java.io.IOException
		{
			out.print("</table>");
			out.print("</td>");
		}
		%>
		<!-- main menu -->
		<table width="100%" id="MAIN_MENU_TABLE" border="0"  cellspacing="0" cellpadding="0" class="main_menu">
		<tr>
			<td background="/images/smsc_14.jpg" width="26"></td>
			<%
/*
			main_menu_begin_item(out, "MENU0_SMSC_menu", "MENU0_SMSC_submenu", "menu.smsc");
			main_menu_submenu_item(out, "/smsc_service",               "MENU0_SMSC",             "menu.smsc.configuration");
			main_menu_submenu_item(out, "/reshedule",                  "MENU0_SMSC_Reshedule",   "menu.smsc.reschedule");
			main_menu_submenu_item(out, "/directives",                 "MENU0_SMSC_Directives",  "menu.smsc.directives");
			main_menu_submenu_i_fu(out, "/smsc_service/status.jsp",    "MENU0_SMSC_Status",      "menu.smsc.status");
			main_menu_submenu_i_fu(out, "/smsc_service/logging.jsp",   "MENU0_SMSC_LOGGING",     "menu.smsc.logging");
			main_menu_submenu_item(out, "/users",                      "MENU0_USERS",            "menu.smsc.users");
			main_menu_submenu_item(out, "/locale_resources",           "MENU0_LOCALE_RESOURCES", "menu.smsc.resources");
			main_menu_submenu_item(out, "/journal",                    "MENU0_JOURNAL",          "menu.smsc.journal");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Statistics_menu", "MENU0_Statistics_submenu", "menu.stat");
			main_menu_submenu_item(out, "/perfmon",            "MENU0_PERFMON"  , "menu.stat.perfmon", "SMSC Performance monitor", "return openPerfMon();");
			main_menu_submenu_item(out, "/topmon",            "MENU0_TOPMON"  ,   "menu.stat.topmon",  "SMSC Sme Monitor", "return openTopMon();");
			main_menu_submenu_item(out, "/smsstat",            "MENU0_SMSSTAT",   "menu.stat.smsstat");
			main_menu_submenu_i_fu(out, "/smsstat/export.jsp", "MENU0_SMSSTAT",   "menu.stat.statexport");
			main_menu_submenu_i_fu(out, "/smsview/daemon.jsp", "MENU0_SMSDAEMON", "menu.stat.daemon");
			main_menu_submenu_i_fu(out, "/smsview/index.jsp" , "MENU0_SMSVIEW",   "menu.stat.smsview");
			main_menu_submenu_i_fu(out, "/smsview/export.jsp" ,"MENU0_SMSEXPORT", "menu.stat.smsexport");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Routing_menu", "MENU0_Routing_submenu", "menu.routing");
			main_menu_submenu_item(out, "/aliases",  "MENU0_ALIASES",  "menu.routing.aliases");
			main_menu_submenu_item(out, "/subjects", "MENU0_SUBJECTS", "menu.routing.subjects");
			main_menu_submenu_item(out, "/routes",   "MENU0_ROUTES",   "menu.routing.routes");
			main_menu_submenu_item(out, "/providers",   "MENU0_PROVIDERS",   "menu.routing.providers");
			main_menu_submenu_item(out, "/categories",   "MENU0_CATEGORIES", "menu.routing.categories");
			main_menu_submenu_item(out, "/acl",      "MENU0_ACL",      "menu.routing.acl");
			main_menu_submenu_item(out, "/tracer",   "MENU0_TRACER",   "menu.routing.tracer");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Profiles_menu", "MENU0_Profiles_submenu", "menu.profiles");
			main_menu_submenu_item(out, "/profiles",            "MENU0_PROFILES",        "menu.profiles.list");
			main_menu_submenu_i_fu(out, "/profiles/groups.jsp", "MENU0_PROFILES_GROUPS", "menu.profiles.groups");
			main_menu_submenu_i_fu(out, "/profiles/lookup.jsp", "MENU0_PROFILES_LOOKUP", "menu.profiles.lookup");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Services_menu", "MENU0_Services_submenu", "menu.services");
			main_menu_submenu_item(out, "/hosts",    "MENU0_HOSTS",    "menu.services.hosts");
			main_menu_submenu_item(out, "/services", "MENU0_SERVICES", "menu.services.services");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Options_menu", "MENU0_Options_submenu", "menu.options");
			main_menu_submenu_item(out, "/mscman",            "MENU0_MSCMAN",        "menu.options.mscs");
			main_menu_submenu_item(out, "/dl",                "MENU0_DL",            "menu.options.dl");
			main_menu_submenu_i_fu(out, "/dl/principals.jsp", "MENU0_DL_PRINCIPALS", "menu.options.principals");
			main_menu_end_item(out);

			main_menu_begin_item(out, "MENU0_Help_menu", "MENU0_Help_submenu", "menu.help");
			main_menu_submenu_i_fu(out, "/admin_guide.jsp",            "MENU0_ADMINGUIDE", "menu.help.adminguide");
			main_menu_end_item(out);
            */
			%>
			<td background="/images/smsc_14.jpg" width="100%">&nbsp;</td>
            <%
               if (request.getUserPrincipal() != null)
                out.print("<td background=\"/images/smsc_14.jpg\"><a ID=\"MENU0_LOGOUT\" href=\"logout.jsp\">"+getLocString("menu.logout")+"</a></td>");
            %>
			<td background="/images/smsc_14.jpg" width="52"></td>
		</tr>
		</table>

<%--		<table width="100%" id="MAIN_MENU_BACKGROUND_TABLE" border="0" cellspacing="0" cellpadding="0" class="main_menu_background">--%>
<%--			<tr>--%>
<%--				<td width="80" background="/images/smsc_13.jpg"></td>--%>
<%--				<td background="/images/smsc_14.jpg">&nbsp;</td>--%>
<%--				<td width="80" background="/images/smsc_16.jpg"></td>--%>
<%--			</tr>--%>
<%--		</table>--%>
		<span id="MENU0_NONE"></span>
		<script>document.all.MENU0_NONE.className = 'submenu_sel';</script>


