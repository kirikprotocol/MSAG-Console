<%@ page import="java.io.IOException,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  java.util.Collection,
					  java.util.Iterator"%><script src="<%=CPATH%>/collapsing_tree.js" type="text/javascript"></script><%!
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
	void param(JspWriter out, String label, String id, String value, String rowId, String additional, boolean isWide) throws IOException
	{
		param(out, label, id, value, rowId, additional, isWide, null);
	}
	void param(JspWriter out, String label, String id, String value, String rowId, String additional, boolean isWide, String validation) throws IOException
	{
	  final boolean additionalPresented = additional != null && additional.length() > 0;
	  out.print("<tr class=row" + ((row++) & 1) + (rowId == null || rowId.length() == 0 ? "" : " id=\"" + rowId + '"') + ">");
	  out.print("<th nowrap>" + label + ":</th>");
	  out.print("<td nowrap><input class=txt" + (isWide ? "W" : "") + " name=\"" + id + "\" id=\"" + id + "\" value=\"" + StringEncoderDecoder.encode(value) + "\""
					+ (validation != null && validation.length() > 0 ? " validation=\"" + validation + "\" onkeyup=\"resetValidation(this)\"" : "") + "></td>");
	  if (additionalPresented)
		  out.print("<td>" + additional + "</td>");
	  out.print("</tr>");
	}
	void param(JspWriter out, String label, String id, String value) throws IOException
	{
		param(out, label, id, value, null, null, false);
	}
	void param(JspWriter out, String label, String id, int value) throws IOException
	{
      param(out, label, id, String.valueOf(value));
	}
	void param(JspWriter out, String label, String id, boolean value) throws IOException
	{
		param(out, label, id, value, null);
	}
	void param(JspWriter out, String label, String id, boolean value, String rowId) throws IOException
	{
		out.print("<tr class=row" + ((row++) & 1) + (rowId != null && rowId.length() > 0 ? " id=\"" + rowId + "\"" : "") + ">");
		out.print("<th nowrap><label for=\"" + id + "\">" + label + ":</label></th>");
		out.print("<td><input class=check type=checkbox name=\"" + id + "\" id=\"" + id + "\" value=true " + (value ? "checked" : "") + "></td>");
		out.print("</tr>");
	}
	void param(JspWriter out, String label, String id, String value, boolean isWide) throws IOException
	{
		param(out, label, id, value, null, null, isWide);
	}
	void param(JspWriter out, String label, String id, String value, String rowId, String additional) throws IOException
	{
	  param(out, label, id, value, rowId, additional, additional != null && additional.length() > 0);
	}
	void finishParams(JspWriter out) throws IOException
	{
		out.print("</table>");
	}
	void paramSelect(JspWriter out, String label, String id, Collection values, String selectedValue) throws IOException
	{
		paramSelect(out, label, id, values, selectedValue, null, null);
	}
  void paramSelect(JspWriter out, String label, String id, Collection values, String selectedValue, String onChange) throws IOException
  {
    paramSelect(out, label, id, values, selectedValue, onChange, null);
  }
	void paramSelect(JspWriter out, String label, String id, Collection values, String selectedValue, String onChange, String style) throws IOException
	{
		out.print("<tr class=row" + ((row++) & 1) + ">");
		out.print("<th nowrap>" + label + ":</th>");
		out.print("<td><select name=\"" + id + "\" id=\"" + id + "\""
              + (onChange != null && onChange.length() > 0 ? " onchange=\"" + onChange + "\"" : "")
              + (style    != null && style   .length() > 0 ? " style=\""    + style    + "\"" : "")
              + ">");
		for (Iterator i = values.iterator(); i.hasNext();) {
			String value = (String) i.next();
			final String encValue = StringEncoderDecoder.encode(value);
			out.print("<option value=\"" + encValue + "\"");
			if (selectedValue != null && selectedValue.equals(value))
				out.print(" selected");
			out.print(">" + encValue + "</option>");
		}
		out.print("</select></td>");
		out.print("</tr>");
	}
	void paramTextarea(JspWriter out, String label, String id, String value) throws IOException
	{
	  out.print("<tr class=row" + ((row++) & 1) + ">");
	  out.print("<th nowrap>" + label + ":</th>");
	  out.print("<td><textarea name=\"" + id + "\">" + StringEncoderDecoder.encode(value) + "</textarea></td>");
	  out.print("</tr>");
	}
%>