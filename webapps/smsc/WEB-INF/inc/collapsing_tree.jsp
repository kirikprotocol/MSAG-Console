<%@ page import="java.io.IOException,
					  ru.novosoft.smsc.util.StringEncoderDecoder"%><script src="<%=CPATH%>/collapsing_tree.js" type="text/javascript"></script><%!
	int row = 0;
	void startSection(JspWriter out, String sectionId, String sectionName, boolean opened) throws IOException
	{
		startSection(out, sectionId, sectionName, opened, null);
	}
	void startSection(JspWriter out, String sectionId, String sectionName, boolean opened, String additional) throws IOException
	{
		final boolean additionalPresented = additional != null && additional.length() > 0;
		out.print("<div class=" + (opened ? "collapsing_tree_opened" : "collapsing_tree_closed") + "  id=\"sectionHeader_" + sectionId + "\" onclick=\"collasping_tree_showhide_section('" + sectionId + "')\">");
		if (additionalPresented)
			out.print("<table cellspacing=0><tr><td width=100% >");
		out.print(sectionName);
		if (additionalPresented)
		{
			out.print("</td>");
			out.print("<td>" + additional + "</td>");
			out.print("</tr></table>");
		}
		out.print("</div>");

		out.print("<table cellspacing=0 cellpadding=0 id=\"sectionValue_" + sectionId + "\" " + (opened ? "" : "style=\"display:none\"") + ">");
		out.print("<col width='56px'/>");

		out.print("<tr><th/><td>");
	}
	void continueSection(JspWriter out) throws IOException
	{
		out.print("</td></tr><tr><th/><td>");
	}
	void finishSection(JspWriter out) throws IOException
	{
		out.print("</td></tr>");
		out.print("</table>");
	}
	void startParams(JspWriter out, String id) throws IOException
	{
		row = 0;
		out.print("<table class=properties_list cellspacing=0 " + (id == null || id.length() == 0 ? "" : " id=\"" + id + '"') + ">");
		out.print("<col width=150px>");
	}
	void startParams(JspWriter out) throws IOException
	{
		startParams(out, null);
	}
	void param(JspWriter out, String label, String id, String value, String rowId, String additional) throws IOException
	{
		final boolean additionalPresented = additional != null && additional.length() > 0;
		out.print("<tr class=row" + ((row++) & 1) + (rowId == null || rowId.length() == 0 ? "" : " id=\"" + rowId + '"') + ">");
      out.print("<th nowrap>" + label + ":</th>");
		out.print("<td nowrap><input class=txt" + (additionalPresented ? "W" : "") + " name=\"" + id + "\" value=\"" + StringEncoderDecoder.encode(value) + "\"></td>");
		if (additionalPresented)
			out.print("<td>" + additional + "</td>");
		out.print("</tr>");
	}
	void param(JspWriter out, String label, String id, String value) throws IOException
	{
		param(out, label, id, value, null, null);
	}
	void param(JspWriter out, String label, String id, int value) throws IOException
	{
      param(out, label, id, String.valueOf(value));
	}
	void param(JspWriter out, String label, String id, boolean value) throws IOException
	{
		out.print("<tr class=row" + ((row++) & 1) + ">");
		out.print("<th nowrap>" + label + ":</th>");
		out.print("<td><input class=check type=checkbox name=\"" + id + "\" value=true " + (value ? "checked" : "") + "></td>");
		out.print("</tr>");
	}
	void finishParams(JspWriter out) throws IOException
	{
		out.print("</table>");
	}
%>