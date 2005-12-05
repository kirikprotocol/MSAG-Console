<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 java.util.*, ru.novosoft.smsc.util.Functions,
                 ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.infosme.beans.InfoSmeBean"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
    ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
    if (!request.isUserInRole(InfoSmeBean.INFOSME_ADMIN_ROLE)) {
        response.sendRedirect("deliveries.jsp");
        return;
    }
    int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<OBJECT id="tdcTasksStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="/smsc/smsc/esme_InfoSme/taskStatuses.jsp">
	<PARAM NAME="UseHeader" VALUE="True">
	<PARAM NAME="TextQualifier" VALUE='"'>
</OBJECT><script type="text/javascript">
function refreshTaskStatuses()
{
	document.all.tdcTasksStatuses.DataURL = document.all.tdcTasksStatuses.DataURL;
	document.all.tdcTasksStatuses.reset();
	window.setTimeout(refreshTaskStatuses, 5000);
}
refreshTaskStatuses();
</script>
<div class=content>
<script type="text/javascript">
function checkApplyResetButtons()
{
  opForm.all.mbApply.disabled = !(opForm.all.allCheck.checked || opForm.all.tasksCheck.checked || opForm.all.schedsCheck.checked);
  opForm.all.mbReset.disabled = !(   (<%=bean.isChangedAll()%> && opForm.all.allCheck.checked)
                                  || (<%=bean.isChangedTasks()%> && opForm.all.tasksCheck.checked)
                                  || (<%=bean.isChangedShedules()%> && opForm.all.schedsCheck.checked)
                                 );
}
</script>
<%! private String configStatus(boolean isChanged) {
    return (isChanged) ? ("<span style='color:red;'>"+getLocString("infosme.label.changed")+"</span>"):
                         getLocString("infosme.label.clear");
}%>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=edit>
<input type=hidden name=sort>
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
      <td><%=serviceStatus(bean.getAppContext(), bean.getSmeId(), "InfoSme_RUNNING_STATUSERVICE_InfoSme")%></td>
    </tr>
    <tr class=row1 id=procRow>
      <td style="padding-right:3px"><input class=check type=checkbox id=toStartProcessor name=toStart value=processor onClick="checkStartStop();"></td>
      <th><label for=toStartProcessor><%= getLocString("infosme.label.send_processor")%><label></th>
      <td id=procStatus><%=bean.isTaskProcessorRuning() ? "running" : "stopped"%></td>
    </tr>
    <tr class=row0 id=schedRow>
      <td style="padding-right:3px"><input class=check type=checkbox id=toStartScheduler name=toStart value=scheduler onClick="checkStartStop();"></td>
      <th><label for=toStartScheduler><%= getLocString("infosme.label.gen_processor")%></label></th>
      <td id=schedStatus><%=bean.isTaskSchedulerRuning() ? "running" : "stopped"%></td>
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
      <td><input class=check type=checkbox name=apply value=tasks id=tasksCheck onClick="checkApplyResetButtons();" <%=bean.isInfosmeStarted() && bean.isChangedTasks() && !bean.isChangedAll() ? "" : "disabled"%>></td>
      <th><label for=tasksCheck><%= getLocString("infosme.label.task_config")%></label></th>
      <td><%=configStatus(bean.isChangedTasks())%></td>
    </tr>
    <tr class=row0 id=schedsRow>
      <td><input class=check type=checkbox name=apply value=scheds id=schedsCheck onCLick="checkApplyResetButtons();" <%=bean.isInfosmeStarted() && bean.isChangedShedules() && !bean.isChangedAll() && !bean.isChangedTasks() ? "" : "disabled"%>></td>
      <th><label for=schedsCheck><%= getLocString("infosme.label.sched_config")%></label></th>
      <td><%=configStatus(bean.isChangedShedules())%></td>
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
page_menu_button(session, out, "mbReset",  "common.buttons.reset", "infosme.hint.reset_changes", bean.isChangedAll() || bean.isChangedTasks() || bean.isChangedShedules());
page_menu_end(out);
%>
<script type="text/javascript">
function checkStartStop()
{
  var status = document.all.InfoSme_RUNNING_STATUSERVICE_InfoSme.innerText;
  var smeRunning = (status == "<%= getLocString("common.statuses.online1") %>" ||
                    status == "<%= getLocString("common.statuses.online2") %>" ||
                    status == "<%= getLocString("common.statuses.running") %>" ||
                    status == "<%= getLocString("common.statuses.stopping")%>" ||
                    status == "<%= getLocString("common.statuses.starting")%>" ||
                    status == "<%= getLocString("common.statuses.unknown" )%>" );
  var smeStopped  = (status == "<%= getLocString("common.statuses.offline") %>" ||
                    status == "<%= getLocString("common.statuses.stopped") %>" ||
                    status == "<%= getLocString("common.statuses.stopping")%>" ||
                    status == "<%= getLocString("common.statuses.unknown") %>" );

  var smeChecked = opForm.all.toStartSme.checked;
  var procChecked = opForm.all.toStartProcessor.checked;
  var procRunning = opForm.all.procStatus.innerText == "running";
  var schedChecked = opForm.all.toStartScheduler.checked;
  var schedRunning = opForm.all.schedStatus.innerText == "running";

  var smeStart = smeStopped && smeChecked;
  var smeStop = smeRunning && smeChecked;
  var procStart = (!procRunning) && procChecked;
  var procStop = procRunning && procChecked;
  var schedStart = (!schedRunning) && schedChecked;
  var schedStop = schedRunning && schedChecked;

  var start = smeStart || (smeRunning && (procStart || schedStart));
  var stop = smeStop || (smeRunning && (procStop || schedStop));

  opForm.all.mbStart.disabled = !start;
  opForm.all.mbStop.disabled = !stop;

  opForm.all.startStopTable.rows.procRow.runtimeStyle.display = smeRunning ? "block" : "none";
  opForm.all.startStopTable.rows.schedRow.runtimeStyle.display = smeRunning ? "block" : "none";

  opForm.all.toStartProcessor.disabled = smeChecked;
  opForm.all.toStartScheduler.disabled = smeChecked;

  window.setTimeout(checkStartStop, 500);
}
checkStartStop();

function setSort(sorting)
{
	if (sorting == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = sorting;
	opForm.submit();
	return false;
}
</script><div class=content>
<%
  QueryResultSet tasks = bean.getTasks();
  if (tasks.size() == 0)
  {%>No tasks found<%}
  else
  {%>
    <table class=list cellspacing=0>
    <col width="1%"             > <!--checkbox-->
    <col width="1%" align=center> <!--enabled-->
    <col width="94%"            > <!--name-->
    <col width="1%" align=center> <!--generating-->
    <col width="1%" align=center> <!--processing-->
    <col width="1%" align=center> <!--priority-->
    <col width="1%" align=center> <!--trackIntegrity-->
    <tr>
      <th>&nbsp;</th>
      <th><a href="#" <%=bean.getSort().endsWith("enabled")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("infosme.label.sort_enabled")%>"   onclick='return setSort("enabled")'       ><%= getLocString("infosme.label.on")%></a></th>
      <th><a href="#" <%=bean.getSort().endsWith("name")           ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("infosme.label.sort_task_name")%>" onclick='return setSort("name")'          ><%= getLocString("infosme.label.task")%></a></th>
      <th><a href="#" <%=bean.getSort().endsWith("generating")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("infosme.label.sort_gen_flag")%>"  onclick='return setSort("generating")'    ><%= getLocString("infosme.label.generating")%></a></th>
      <th><a href="#" <%=bean.getSort().endsWith("processing")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("infosme.label.sort_proc_flag")%>" onclick='return setSort("processing")'    ><%= getLocString("infosme.label.sending")%></a></th>
      <th><a href="#" <%=bean.getSort().endsWith("priority")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("infosme.label.sort_priority")%>"  onclick='return setSort("priority")'      ><%= getLocString("infosme.label.priority")%></a></th>
      <th><a href="#" <%=bean.getSort().endsWith("trackIntegrity") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%= getLocString("infosme.label.sort_integrity")%>" onclick='return setSort("trackIntegrity")'><%= getLocString("infosme.label.integrity")%></a></th>
    </tr>
    <%
    int rowN = 0;
    for (Iterator i = tasks.iterator(); i.hasNext();) {
      TaskDataItem task = (TaskDataItem) i.next();

      String id = task.getId();
      String name = task.getName();
      //String provider = task.getProvider();
      int priority = task.getPriority();
      boolean enabled = task.isEnabled();
      boolean generating = task.isGenerating();
      boolean processing = task.isProcessing();
      //boolean retryOnFail = task.isRetryOnFail();
      //boolean replaceMessage = task.isReplaceMessage();
      boolean trackIntegrity = task.isTrackIntegrity();
      //String svcType = task.getSvcType();

      String idEnc = StringEncoderDecoder.encode(id);
      String idHex = StringEncoderDecoder.encodeHEX(id);
      String nameEnc = StringEncoderDecoder.encode(name);
      //String providerEnc = StringEncoderDecoder.encode(provider);
      //String svcTypeEnc = StringEncoderDecoder.encode(svcType);

      %><tr class=row<%=rowN++&1%>>
        <td><input class=check type=checkbox name=checked id=checked<%=idHex%> value="<%=idEnc%>" <%=bean.isTaskChecked(id) ? "checked" : ""%> onclick="checkTasks();"></td>
        <td><%if (enabled){%><img src="/images/ic_checked.gif"><%}else{%><img src="/images/ic_stopped.gif"><%}%></td>
        <td><label for=checked<%=idHex%>><%=nameEnc%></label></td>
        <td><span datasrc=#tdcTasksStatuses DATAFORMATAS=html datafld="gen<%=idHex%>"><%if (generating){%><img src="/images/ic_running.gif"><%}else{%><img src="/images/ic_stopped.gif"><%}%></span></td>
        <td><span datasrc=#tdcTasksStatuses DATAFORMATAS=html datafld="prc<%=idHex%>"><%if (processing){%><img src="/images/ic_running.gif"><%}else{%><img src="/images/ic_stopped.gif"><%}%></span></td>
        <td><%=priority%></td>
        <td><%if (trackIntegrity){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
      </tr><%
    }
    %></table><%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%><%
  }%>
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
function checkTasks()
{
  var prefix = "checked";
  var prefixLen = prefix.length;
  var inputs = opForm.getElementsByTagName("input");
  var disabledElem = true;
  for (i = 0; i < inputs.length; i++)
  {
    var inp = inputs[i];
    if (inp.type == "checkbox" && inp.id != null && inp.id.substring(0, prefixLen) == prefix)
      disabledElem &= !inp.checked;
  }
  document.all.mbEnableTask.disabled = document.all.mbDisableTask.disabled
    = document.all.mbStartTask.disabled = document.all.mbStopTask.disabled = disabledElem;
  return true;
}
checkTasks();
checkCheckboxes(opForm.all.mbApply);
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>