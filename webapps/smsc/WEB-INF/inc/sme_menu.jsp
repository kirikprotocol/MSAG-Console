<%@ page import="java.io.IOException"%><%!
private boolean sme_menu_delimiter_needed = false;
void sme_menu_begin(JspWriter out) throws IOException
{
   sme_menu_delimiter_needed = false;
	out.print("<table class=sme_menu cellpadding=0px cellspacing=0px>");
	out.print("<tr>");
	out.print("<th width=12px background=\"" + CPATH + "/img/smsc_35.jpg\">&nbsp;</th>");
	out.print("<th width=12px>&nbsp;</th>");
}
void sme_menu_button(JspWriter out, String name, String value, String visibleName, String title) throws IOException
{
	sme_menu_button(out, name, value, visibleName, title, null, true);
}
void sme_menu_button(JspWriter out, String name, String value, String visibleName, String title, String onclick) throws IOException
{
	sme_menu_button(out, name, value, visibleName, title, onclick, true);
}
void sme_menu_button(JspWriter out, String name, String value, String visibleName, String title, boolean enabled) throws IOException
{
	sme_menu_button(out, name, value, visibleName, title, null, enabled);
}
void sme_menu_button(JspWriter out, String name, String value, String visibleName, String title, String onclick, boolean enabled) throws IOException
{
	if (sme_menu_delimiter_needed)
		out.print("<td width=1px>|</td>");
	out.print("<td width=1px>");
	//out.print("<input type=submit id=\"" + name + "\" name=\"" + name + "\" value=\"" + value + "\" title=\"" + title + "\"" + (enabled ? "" : " disabled") + ((onclick != null && onclick.length() > 0) ? (" onclick=\"" + onclick + '"') : "" ) +">");
	out.print("<a id=\"" + name + "\" jbuttonName=\"" + name + "\" jbuttonValue=\"" + value + "\""
				+ (enabled ? "" : " class=disabled")
				+ ((title != null && title.length() > 0) ? " title=\"" + title + "\"" : "")
				+ ((onclick != null && onclick.length() > 0) ? (" jbuttonOnclick=\"" + onclick + '"') : "" ) 
				+">" + visibleName + "</a>");
	out.print("</td>");
	sme_menu_delimiter_needed = true;
}
void sme_menu_space(JspWriter out) throws IOException
{
	out.print("<td>&nbsp;</td>");
	sme_menu_delimiter_needed = false;
}
void sme_menu_end(JspWriter out) throws IOException
{
	out.print("<th width=12px background=\"" + CPATH + "/img/smsc_37.jpg\">&nbsp;</th>");
	out.print("</tr>");
	out.print("</table>");

	sme_menu_delimiter_needed = false;
}
%>