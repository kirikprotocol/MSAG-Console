<%@ page import="ru.novosoft.smsc.infosme.beans.deliveries.DeliveriesPageData"%>
<div class=content>
  <div class=page_subtitle><%= getLocString("infosme.subtitle.stage5")%></div><br/>
  <div class=secInfo><%= ((deliveries_bean.getGenerationStatus() == DeliveriesPageData.STATUS_DONE) ?
      getLocString("infosme.msg.task_add_success"):
      getLocString("infosme.msg.task_add_failure"))%>'<%= deliveries_bean.getName()%>': <%=" " + deliveries_bean.getStatusStr()%></div>
  <div class=secInfo><%= getLocString("infosme.msg.to_main")%></div>
</div>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbNext", "common.buttons.done", "infosme.hint.main_page");
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "infosme.hint.main_page");
  page_menu_space(out);
  page_menu_end(out);
%>