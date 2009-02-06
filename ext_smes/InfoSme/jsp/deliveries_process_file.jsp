<script type="text/javascript">
  var statusDataSource = new StringTableDataSource({url: '/smsc/smsc/esme_InfoSme/deliveries_process_progress.jsp', async: false});
</script>

<div class=content>

  <div class=page_subtitle><%= getLocString("infosme.subtitle.stage2")%></div><br/>
  <div class=secInfo><%= getLocString("infosme.label.status")%>&nbsp;<span id="tdcStatus" style='color:blue;'><%= deliveries_bean.getStatusStr()%></span></div>
  <div>
    <div class=secInfo><%= getLocString("Records Processed")%>&nbsp;<span id="tdcRecordsProcessed" ><%= deliveries_bean.getRecordsProcessed()%></span></div>
    <div class=secInfo><%= getLocString("Regions found")%>&nbsp;<span id="tdcRegionsFound" ><%= deliveries_bean.getRegionsFound()%></span></div>
    <div class=secInfo><%= getLocString("Unrecognized numbers")%>&nbsp;<span id="tdcUnrecognized" ><%= deliveries_bean.getUnrecognized()%></span></div>
    <div class=secInfo><%= getLocString("Black numbers")%>&nbsp;<span id="tdcInBlackList" ><%= deliveries_bean.getInBlackList()%></span></div>
  </div>
</div>

<script language="javascript" type="text/javascript">
  statusDataSource.hasObservers();
  statusDataSource.addObserver(new ElementObserver({elementId: 'tdcStatus', field: 'status'}));
  statusDataSource.addObserver(new ElementObserver({elementId: 'tdcRecordsProcessed', field: 'recordsProcessed'}));
  statusDataSource.addObserver(new ElementObserver({elementId: 'tdcRegionsFound', field: 'regionsFound'}));
  statusDataSource.addObserver(new ElementObserver({elementId: 'tdcUnrecognized', field: 'unrecognized'}));
  statusDataSource.addObserver(new ElementObserver({elementId: 'tdcInBlackList', field: 'inBlackList'}));

  function refreshProgressStatus() {    
    statusDataSource.update();
    window.setTimeout(refreshProgressStatus, 5000);
  }

  window.setTimeout(refreshProgressStatus, 5000);
</script>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel",    "infosme.hint.cancel_task_add", "clickCancel()");
  page_menu_button(session, out, "mbNext",   "infosme.button.next_page", "infosme.hint.finish_task_add", true);
  page_menu_space(out);
  page_menu_end(out);
%>