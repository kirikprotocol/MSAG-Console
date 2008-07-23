<%@ page import="java.util.Iterator"%>
<script type="text/javascript">
  var statusDataSource = new StringTableDataSource({url: '/smsc/smsc/esme_InfoSme/deliveries_gen_progress.jsp', async: false});
</script>

<div class=content>

  <div class=page_subtitle><%= getLocString("infosme.subtitle.stage4")%></div><br/>
  <div class=secInfo><%= getLocString("infosme.label.status")%>&nbsp;<span id="tdcStatus" style='color:blue;'><%= deliveries_bean.getStatusStr()%></span></div>

  <%
    for (Iterator iter = deliveries_bean.getSubjects().iterator(); iter.hasNext();) {
      String subject = (String)iter.next();
  %>
  <div>&nbsp;</div>
  <div class=page_subtitle><%=subject%>:</div>
  <div>
    <div class=secInfo><%= getLocString("infosme.label.messages_generated")%>&nbsp;<span id="tdcMessages<%=subject%>" ><%= deliveries_bean.getMessages(subject)%></span></div>
    <div class=secInfo><%= getLocString("infosme.label.total_progress")%>&nbsp;<span id="tdcProcents<%=subject%>" ><%= deliveries_bean.getProgress(subject)%></span>%</div>
  </div>
  <script type="text/javascript">
    statusDataSource.addObserver(new ElementObserver({elementId: 'tdcMessages<%=subject%>', field: 'messages<%=subject%>'}));
    statusDataSource.addObserver(new ElementObserver({elementId: 'tdcProcents<%=subject%>', field: 'progress<%=subject%>'}));
  </script>
  <%}%>
</div>

<script type="text/javascript">
  statusDataSource.addObserver(new ElementObserver({elementId: 'tdcStatus', field: 'status'}));

  function refreshProgressStatus() {
    statusDataSource.update();
    window.setTimeout(refreshProgressStatus, 5000);
  }
  refreshProgressStatus();
</script>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel",    "infosme.hint.cancel_task_add", "clickCancel()");
  page_menu_button(session, out, "mbNext",   "infosme.button.next_page", "infosme.hint.finish_task_add", true);
  page_menu_space(out);
  page_menu_end(out);
%>