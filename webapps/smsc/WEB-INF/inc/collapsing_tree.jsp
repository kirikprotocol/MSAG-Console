<script src="collapsing_tree.js" type="text/javascript"></script><%!
	int row = 0;
	void startSection(JspWriter out, String sectionId, String sectionName, boolean opened) throws IOException
	{
		out.print("<div class=" + (opened ? "collapsing_tree_opened" : "collapsing_tree_closed") + "  id=\"" + sectionId + "_header\" onclick=\"collasping_tree_showhide_section('" + sectionId + "')\">");
		out.print(sectionName);
		out.print("</div>");

		out.print("<table cellspacing=0 cellpadding=0 id=\"" + sectionId + "_value\" " + (opened ? "" : "style=\"display:none\"") + ">");
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
	void startParams(JspWriter out) throws IOException
	{
		row = 0;
		out.print("<table cellspacing=0 cellpadding=0>");
		out.print("<col width=150px>");
	}
	void param(JspWriter out, String label, String id, String value) throws IOException
	{
		out.print("<tr class=row" + ((row++) & 1) + ">");
      out.print("<th nowrap>" + label + ":</th>");
		out.print("<td><input class=txt name=\"" + id + "\" value=\"" + StringEncoderDecoder.encode(value) + "\"></td>");
		out.print("</tr>");
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