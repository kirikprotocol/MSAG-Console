<div class=content>
  <div class=page_subtitle><%= getLocString("infosme.subtitle.welcome")%></div><br/>
  <div class=secInfo><%= getLocString("infosme.label.phraze1")%></div>
  <div class=secInfo><%= getLocString("infosme.label.phraze2")%></div>
  <div class=secInfo><%= getLocString("infosme.label.phraze3")%></div>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbNext",   "infosme.button.create_delivery",     "infosme.hint.create_delivery");
  page_menu_button(session, out, "mbStat",   "infosme.button.view_statistics",     "infosme.hint.view_statistics");
  page_menu_button(session, out, "mbDlstat", "infosme.button.download_statistics", "infosme.hint.download_statistics");
  page_menu_space(out);
  page_menu_end(out);
%>