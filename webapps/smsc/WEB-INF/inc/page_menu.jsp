<%@ page import="java.io.IOException" %>
<%!
  private boolean page_menu_delimiter_needed = false;
  void page_menu_begin(JspWriter out) throws IOException {
    page_menu_delimiter_needed = false;

    out.print("<div class=\"page_menu\">");
    out.print("<table class=\"page_menu\" cellpadding=\"0px\" cellspacing=\"0px\">");
    out.print("<tr>");
    out.print("<td width=\"37px\">&nbsp;</td>");
  }

  void page_small_menu_begin(JspWriter out) throws IOException {
    page_menu_delimiter_needed = false;


            //out.print("<div class=page_menu>");
    //out.print("<table class=page_menu cellpadding=0px cellspacing=0px>");
    //	out.print("<tr>");
    out.print("<td width=\"1px\">&nbsp;</td>");
  }

  void page_menu_button(HttpSession session, JspWriter out, String name, String value, String title) throws IOException {
    page_menu_button(session, out, name, value, title, null, true);
  }

  void page_menu_button(HttpSession session, JspWriter out, String name, String value, String title, String onclick) throws
          IOException {
    page_menu_button(session, out, name, value, title, onclick, true);
  }

  void page_menu_button(HttpSession session, JspWriter out, String name, String value, String title, boolean enabled) throws
          IOException {
    page_menu_button(session, out, name, value, title, null, enabled);
  }

  void page_menu_button(HttpSession session, JspWriter out, String name, String value, String title, String onclick, boolean
          enabled) throws IOException {
    if (page_menu_delimiter_needed) {
      out.print("<td width=\"1px\">|</td>");
    }

    out.print("<td width=\"1px\">");
    out.print("<a id=\"" + name + "\"" +
      (enabled ? "" : " disabled") +
      ((title != null && title.length() > 0) ? " title=\"" + getLocString(title) + "\"" : "") +
      ((onclick != null && onclick.length() > 0) ? makeHref(onclick) : makeHref("clickSubmit('" + name + "','" + getLocString(value) + "')")) +
      ">" + getLocString(value) + "</a>");
    out.print("</td>");

    page_menu_delimiter_needed = true;
  }

  void page_menu_confirm_button(HttpSession session, JspWriter out, String name, String value, String title, String
          confirmation) throws IOException {
    page_menu_confirm_button(session, out, name, value, title, confirmation, null, true);
  }

  void page_menu_confirm_button(HttpSession session, JspWriter out, String name, String value, String title, String
          confirmation, String onclick) throws IOException {
    page_menu_confirm_button(session, out, name, value, title, confirmation, onclick, true);
  }

  void page_menu_confirm_button(HttpSession session, JspWriter out, String name, String value, String title, String
          confirmation, String onclick, boolean enabled) throws IOException {
    if (page_menu_delimiter_needed) {
      out.print("<td width=\"1px\">|</td>");
    }

    out.print("<td width=\"1px\">");
    out.print("<a id=\"" + name + "\"" +
            (enabled ? "" : " disabled") +
            ((title != null && title.length() > 0) ? " title=\"" + getLocString(title) + "\"" : "") +
            ((onclick != null && onclick.length() > 0) ? makeHref( onclick )
              : ((confirmation != null && confirmation.length() > 0) ?
                    makeHref("clickConfirmSubmit('" + name + "','" + getLocString(value) + "', '"+confirmation+"')")
                  : makeHref("clickSubmit('" + name + "','" + getLocString(value) + "')")
                )
            ) +
            ">" + getLocString(value) + "</a>"
            );
    out.print("</td>");

    page_menu_delimiter_needed = true;
  }

  void page_menu_space(JspWriter out) throws IOException {
    out.print("<td>&nbsp;</td>");

    page_menu_delimiter_needed = false;
  }

  void page_menu_end(JspWriter out) throws IOException {
    out.print("<td width=\"37px\">&nbsp;</td>");
    out.print("</tr>");
    out.print("</table>");
    out.print("<table class=\"page_menu_background\" cellspacing=\"0px\" cellpadding=\"0px\">");
    out.print("<tr>");
    out.print("<td width=\"80px\" background=\"/images/smsc_20.jpg\">&nbsp;</td>");
    out.print("<td background=\"/images/smsc_21.jpg\">&nbsp;</td>");
    out.print("<td width=\"80px\" background=\"/images/smsc_23.jpg\">&nbsp;</td>");
    out.print("</tr>");
    out.print("</table>");
    out.print("</div>");

    page_menu_delimiter_needed = false;
  }

  void page_small_menu_end(JspWriter out) throws IOException {
    //	out.print("<td width=1px>&nbsp;</td>");
    //	out.print("</tr>");
    //	out.print("</table>");
    /*	out.print("<table class=page_menu_background cellspacing=0px cellpadding=0px>");
     out.print("<tr>");
     out.print("<td width=80px background=\"/images/smsc_20.jpg\">&nbsp;</td>");
     out.print("<td background=\"/images/smsc_21.jpg\">&nbsp;</td>");
     out.print("<td width=80px background=\"/images/smsc_23.jpg\">&nbsp;</td>");
     out.print("</tr>");
     out.print("</table>");
     */
    //out.print("</div>");
    page_menu_delimiter_needed = false;
  }

%>
