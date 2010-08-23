<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 java.util.*, ru.novosoft.smsc.util.Functions,
                 ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.infosme.beans.InfoSmeBean,
                 java.text.SimpleDateFormat"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper" %>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
    ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
    if (!bean.isUserAdmin(request)) {
        response.sendRedirect("tasks.jsp");
        return;
    }
    int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>

<div class=content>
<script type="text/javascript">
function checkApplyResetButtons()
{
  document.getElementById('mbApply').disabled = !(document.getElementById('allCheck').checked || document.getElementById('tasksCheck').checked || document.getElementById('schedsCheck').checked || document.getElementById('retriesCheck').checked);
  document.getElementById('mbReset').disabled = !(   (<%=bean.isChangedAll()%> && document.getElementById('allCheck').checked)
                                  || (<%=bean.isChangedTasks(request)%> && document.getElementById('tasksCheck').checked)
                                  || (<%=bean.isChangedShedules()%> && document.getElementById('schedsCheck').checked)
                                 );
}
</script>
<%! private String configStatus(boolean isChanged) {
    return (isChanged) ? ("<span style='color:red;'>"+getLocString("infosme.label.changed")+"</span>"):
                         getLocString("infosme.label.clear");
}%>
<table cellspacing=0 cellpadding=0>
<col width="48%">
<col width="4%" >
<col width="48%">
<tr>
  <td valign="top">
    <table class=properties_list cellspacing=0 cellpadding=0 id=startStopTable>
    <col width="1%">
    <col width="79%">
    <col width="20%" align="right">
    <tr class=row0>
      <td style="padding-right:3px"><input class=check type=checkbox id=toStartSme name=toStart value=sme onClick="checkStartStop();"></td>
      <th><label for=toStartSme><%= getLocString("infosme.label.informer_service")%></label></th>
      <td><%=bean.isInfosmeStarted() ? getLocString("common.statuses.online") : getLocString("common.statuses.offline") %></td>
    </tr>
    <tr class=row1 id=procRow>
      <td style="padding-right:3px"><input class=check type=checkbox id=toStartProcessor name=toStart value=processor onClick="checkStartStop();"></td>
      <th><label for=toStartProcessor><%= getLocString("infosme.label.send_processor")%></label></th>
      <td id=procStatus><%=bean.isTaskProcessorRuning() ? getLocString("common.statuses.online") : getLocString("common.statuses.offline") %></td>
    </tr>
    <tr class=row0 id=schedRow>
      <td style="padding-right:3px"><input class=check type=checkbox id=toStartScheduler name=toStart value=scheduler onClick="checkStartStop();"></td>
      <th><label for=toStartScheduler><%= getLocString("infosme.label.gen_processor")%></label></th>
      <td id=schedStatus><%=bean.isTaskSchedulerRuning() ? getLocString("common.statuses.online") : getLocString("common.statuses.offline") %></td>
    </tr>
    <tr class=row1 id=siebelRow>
      <td style="padding-right:3px"><input class=check type=checkbox id=toStartSiebel name=toStart value=siebel onClick="checkStartStop();"></td>
      <th><label for=toStartSiebel><%= getLocString("infosme.label.siebel")%></label></th>
      <td id=siebelStatus><%=bean.isSiebelOnline() ? getLocString("common.statuses.online") : getLocString("common.statuses.offline") %></td>
    </tr>
    <tr class=row0 id=archiveRow>
      <td style="padding-right:3px"><input class=check type=checkbox id=toStartArchive name=toStart value=archiveDaemon onClick="checkStartStop();"></td>
      <th><label for=toStartArchive><%= getLocString("infosme.label.tasks_archive")%></label></th>
      <td id=archiveStatus><%=bean.isTaskArchiveOnline() ? getLocString("common.statuses.online") : getLocString("common.statuses.offline") %></td>
    </tr>

    </table>
  </td>
  <td>&nbsp;</td>
  <td>
    <table class=properties_list cellspacing=0>
    <col width="1%">
    <col width="79%">
    <col width="20%" align="right">
    <tr class=row0>
      <td><input class=check type=checkbox name=apply value=all id=allCheck onClick="checkApplyResetButtons();" <%=bean.isChangedAll() ? "" : "disabled"%>></td>
      <th><label for=allCheck><%= getLocString("infosme.label.all_config")%></label></th>
      <td><%=configStatus(bean.isChangedAll())%></td>
    </tr>
    <tr class=row1 id=tasksRow>
      <td><input class=check type=checkbox name=apply value=tasks id=tasksCheck onClick="checkApplyResetButtons();" <%=bean.isInfosmeStarted() && bean.isChangedTasks(request) && !bean.isChangedAll() ? "" : "disabled"%>></td>
      <th><label for=tasksCheck><%= getLocString("infosme.label.task_config")%></label></th>
      <td><%=configStatus(bean.isChangedTasks(request))%></td>
    </tr>
    <tr class=row0 id=schedsRow>
      <td><input class=check type=checkbox name=apply value=scheds id=schedsCheck onCLick="checkApplyResetButtons();" <%=bean.isInfosmeStarted() && bean.isChangedShedules() ? "" : "disabled"%>></td>
      <th><label for=schedsCheck><%= getLocString("infosme.label.sched_config")%></label></th>
      <td><%=configStatus(bean.isChangedShedules())%></td>
    </tr>
    <tr class=row1 id=retriesRow>
      <td><input class=check type=checkbox name=apply value=retries id=retriesCheck onCLick="checkApplyResetButtons();" <%=bean.isInfosmeStarted() && bean.isChangedRetryPolicies()  ? "" : "disabled"%>></td>
      <th><label for=schedsCheck><%= getLocString("infosme.label.retries_config")%></label></th>
      <td><%=configStatus(bean.isChangedRetryPolicies())%></td>
    </tr>
    </table>
  </td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbStart", "common.buttons.start", "infosme.hint.start_selected");
page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "infosme.hint.stop_selected");
page_menu_space(out);
page_menu_button(session, out, "mbApply",  "common.buttons.apply", "infosme.hint.apply_changes");
page_menu_button(session, out, "mbReset",  "common.buttons.reset", "infosme.hint.reset_changes", bean.isChangedAll() || bean.isChangedTasks(request) || bean.isChangedShedules());
page_menu_end(out);
%>
<script type="text/javascript">
function checkStartStop()
{
  var status = document.getElementById('InfoSme_RUNNING_STATUSERVICE_InfoSme').innerText;
  var smeRunning = (status != "<%= getLocString("common.statuses.offline") %>");
  var smeStopped  = (status == "<%= getLocString("common.statuses.offline") %>");

  var smeChecked = document.getElementById('toStartSme').checked;
  var procChecked = document.getElementById('toStartProcessor').checked;
  var procRunning = document.getElementById('procStatus').innerText != "<%= getLocString("common.statuses.offline") %>";
  var schedChecked = document.getElementById('toStartScheduler').checked;
  var schedRunning = document.getElementById('schedStatus').innerText != "<%= getLocString("common.statuses.offline") %>";

  var smeStart = smeStopped && smeChecked;
  var smeStop = smeRunning && smeChecked;
  var procStart = (!procRunning) && procChecked;
  var procStop = procRunning && procChecked;
  var schedStart = (!schedRunning) && schedChecked;
  var schedStop = schedRunning && schedChecked;

  var start = smeStart || (smeRunning && (procStart || schedStart));
  var stop = smeStop || (smeRunning && (procStop || schedStop));

  document.getElementById('mbStart').disabled = !start;
  document.getElementById('mbStop').disabled = !stop;

  document.getElementById('startStopTable').rows.procRow.style.display = smeRunning ? "block" : "none";
  document.getElementById('startStopTable').rows.schedRow.style.display = smeRunning ? "block" : "none";

  document.getElementById('toStartProcessor').disabled = smeChecked;
  document.getElementById('toStartScheduler').disabled = smeChecked;

  window.setTimeout(checkStartStop, 5000);
}
checkStartStop();


</script>
<div class=content>
<%{final PagedStaticTableHelper tableHelper = bean.getTableHelper();
  tableHelper.fillTable(); %>
<%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>
<%}%>
</div><%
  page_menu_begin(out);
  page_menu_button(session, out, "mbEnableTask",  "infosme.button.enable",    "infosme.hint.enable");
  page_menu_button(session, out, "mbDisableTask", "infosme.button.disable",   "infosme.hint.disable");
  page_menu_button(session, out, "mbStartTask",   "infosme.button.start_gen", "infosme.hint.start_gen");
  page_menu_button(session, out, "mbStopTask",    "infosme.button.stop_gen",  "infosme.hint.stop_gen");
  page_menu_space(out);
  page_menu_end(out);
%>
<script type="text/javascript">
checkCheckboxes(document.getElementById('mbApply'));
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>