<%@ page import="java.io.IOException"%><%!
private boolean page_menu_delimiter_needed = false;
void page_menu_begin(JspWriter out) throws IOException
{
   page_menu_delimiter_needed = false;
	out.print("<div class=page_menu>");
	out.print("<table class=page_menu cellpadding=0px cellspacing=0px>");
	out.print("<tr>");
	out.print("<td width=37px>&nbsp;</td>");
}
void page_menu_button(JspWriter out, String name, String value, String title) throws IOException
{
	page_menu_button(out, name, value, title, null, true);
}
void page_menu_button(JspWriter out, String name, String value, String title, String onclick) throws IOException
{
	page_menu_button(out, name, value, title, onclick, true);
}
void page_menu_button(JspWriter out, String name, String value, String title, boolean enabled) throws IOException
{
	page_menu_button(out, name, value, title, null, enabled);
}
void page_menu_button(JspWriter out, String name, String value, String title, String onclick, boolean enabled) throws IOException
{
	if (page_menu_delimiter_needed)
		out.print("<td width=1px>|</td>");
	out.print("<td width=1px><input type=submit id=\"" + name + "\" name=\"" + name + "\" value=\"" + value + "\" title=\"" + title + "\"" + (enabled ? "" : " disabled") + ((onclick != null && onclick.length() > 0) ? (" onclick=\"" + onclick + '"') : "" ) +"></td>");
	page_menu_delimiter_needed = true;
}
void page_menu_space(JspWriter out) throws IOException
{
	out.print("<td>&nbsp;</td>");
	page_menu_delimiter_needed = false;
}
void page_menu_end(JspWriter out) throws IOException
{
	out.print("<td width=37px>&nbsp;</td>");
	out.print("</tr>");
	out.print("</table>");

	out.print("<table class=page_menu_background cellspacing=0px cellpadding=0px>");
	out.print("<tr>");
	out.print("<td width=80px background=\"" + CPATH + "/img/smsc_20.jpg\">&nbsp;</td>");
	out.print("<td background=\"" + CPATH + "/img/smsc_21.jpg\">&nbsp;</td>");
	out.print("<td width=80px background=\"" + CPATH + "/img/smsc_23.jpg\">&nbsp;</td>");
	out.print("</tr>");
	out.print("</table>");
	out.print("</div>");
	page_menu_delimiter_needed = false;
}
%>