<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.infosme.beans.Index,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 java.util.*,
                 ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
	ServiceIDForShowStatus = Constants.INFO_SME_ID;
	TITLE="Informer SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";
	int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<OBJECT id="tdcTasksStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="/smsc/smsc/esme_InfoSme/taskStatuses.jsp">
	<PARAM NAME="UseHeader" VALUE="True">
	<PARAM NAME="TextQualifier" VALUE='"'>
</OBJECT><script>
function refreshTaskStatuses()
{
	document.all.tdcTasksStatuses.DataURL = document.all.tdcTasksStatuses.DataURL;
	document.all.tdcTasksStatuses.reset();
	window.setTimeout(refreshTaskStatuses, 5000);
}
refreshTaskStatuses();
</script>
<div class=content>
<script>
function checkApplyResetButtons()
{
  opForm.all.mbApply.disabled = !(opForm.all.allCheck.checked || opForm.all.tasksCheck.checked || opForm.all.schedsCheck.checked);
  opForm.all.mbReset.disabled = !(   (<%=bean.isChangedAll()%> && opForm.all.allCheck.checked)
                                  || (<%=bean.isChangedTasks()%> && opForm.all.tasksCheck.checked)
                                  || (<%=bean.isChangedShedules()%> && opForm.all.schedsCheck.checked)
                                 );
}
</script>
<%! private String status(boolean isChanged)
{
  if (isChanged)
    return "<span style='color:red;'>changed</span>";
  else
    return "clear";
}
%>
<table class=properties_list cellspacing=0>
<col width="1%">
<col width="79%">
<col width="20%">
<tr class=row0>
  <td><input class=check type=checkbox name=apply value=all id=allCheck onClick="checkApplyResetButtons();" <%=bean.isChangedAll() ? "" : "disabled"%>></td>
  <th><label for=allCheck>All</label></th>
  <td><%=status(bean.isChangedAll())%></td>
</tr>
<tr class=row1 id=tasksRow>
  <td><input class=check type=checkbox name=apply value=tasks id=tasksCheck onClick="checkApplyResetButtons();" <%=bean.isInfosmeStarted() && bean.isChangedTasks() && !bean.isChangedAll() ? "" : "disabled"%>></td>
  <th><label for=tasksCheck>Tasks</label></th>
  <td><%=status(bean.isChangedTasks())%></td>
</tr>
<tr class=row0 id=schedsRow>
  <td><input class=check type=checkbox name=apply value=scheds id=schedsCheck onCLick="checkApplyResetButtons();" <%=bean.isInfosmeStarted() && bean.isChangedShedules() && !bean.isChangedAll() && !bean.isChangedTasks() ? "" : "disabled"%>></td>
  <th><label for=schedsCheck>Schedules</label></th>
  <td><%=status(bean.isChangedShedules())%></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply changes");
page_menu_button(out, "mbReset",  "Reset",  "Discard changes", bean.isChangedAll() || bean.isChangedTasks() || bean.isChangedShedules());
page_menu_space(out);
page_menu_end(out);
%><div class=content>
<script>checkCheckboxes(opForm.all.mbApply);</script>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=edit>
<input type=hidden name=sort>
<table class=properties_list cellspacing=0 cellpadding=0 id=startStopTable>
<col width="1%">
<col width="79%">
<col width="20%">
<tr class=row0>
  <td style="padding-right:3px"><input class=check type=checkbox id=toStartSme name=toStart value=sme onClick="checkStartStop();"></td>
  <th><label for=toStartSme>Info SME</label></th>
  <td><%=serviceStatus(bean.getAppContext(), Constants.INFO_SME_ID, "InfoSme_RUNNING_STATUSERVICE_InfoSme")%></td>
</tr>
<tr class=row1 id=procRow>
  <td style="padding-right:3px"><input class=check type=checkbox id=toStartProcessor name=toStart value=processor onClick="checkStartStop();"></td>
  <th><label for=toStartProcessor>Task Processor<label></th>
  <td id=procStatus><%=bean.isTaskProcessorRuning() ? "running" : "stopped"%></td>
</tr>
<tr class=row0 id=schedRow>
  <td style="padding-right:3px"><input class=check type=checkbox id=toStartScheduler name=toStart value=scheduler onClick="checkStartStop();"></td>
  <th><label for=toStartScheduler>Task Scheduler</label></th>
  <td id=schedStatus><%=bean.isTaskSchedulerRuning() ? "running" : "stopped"%></td>
</tr>
</table></div><%
  page_menu_begin(out);
  page_menu_button(out, "mbStart", "Start", "");
  page_menu_button(out, "mbStop",  "Stop",  "");
  page_menu_space(out);
  page_menu_end(out);
%><script>
function checkStartStop()
{
  var smeChecked = opForm.all.toStartSme.checked;
  var smeRunning = document.all.InfoSme_RUNNING_STATUSERVICE_InfoSme.innerText == "running";
  var smeStopped = document.all.InfoSme_RUNNING_STATUSERVICE_InfoSme.innerText == "stopped";
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
    <col width="1%">
    <col width="90%"            > <!--name-->
    <col width="1%"             > <!--provider-->
    <col width="1%" align=center> <!--enabled-->
    <col width="1%" align=center> <!--generating-->
    <col width="1%" align=center> <!--processing-->
    <col width="1%"             > <!--priority-->
    <col width="1%" align=center> <!--retryOnFail-->
    <col width="1%" align=center> <!--replaceMessage-->
    <col width="1%" align=center> <!--trackIntegrity-->
    <col width="1%"             > <!--svcType-->
    <tr>
      <th>&nbsp;</th>
      <th><a href="#" <%=bean.getSort().endsWith("name")           ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by task name"           onclick='return setSort("name")'          >Task name</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("provider")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by data provider name"  onclick='return setSort("provider")'      >Data provider</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("enabled")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by Enabled flag"        onclick='return setSort("enabled")'       >Enabled</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("generating")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by generating flag"     onclick='return setSort("generating")'    >Generating</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("processing")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by processing flag"     onclick='return setSort("processing")'    >Processing</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("priority")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by priority"            onclick='return setSort("priority")'      >Priority</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("retryOnFail")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by RetryOnFail flag"    onclick='return setSort("retryOnFail")'   >Retry on fail</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("replaceMessage") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by ReplaceMessage flag" onclick='return setSort("replaceMessage")'>Replace message</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("trackIntegrity") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by TrackIntegrity flag" onclick='return setSort("trackIntegrity")'>Track Integrity</a></th>
      <th><a href="#" <%=bean.getSort().endsWith("svcType")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by service type"        onclick='return setSort("svcType")'       >Service type</a></th>
    </tr>
    <%
    int rowN = 0;
    for (Iterator i = tasks.iterator(); i.hasNext();) {
      TaskDataItem task = (TaskDataItem) i.next();

      String id = task.getId();
      String name = task.getName();
      String provider = task.getProvider();
      int priority = task.getPriority();
      boolean enabled = task.isEnabled();
      boolean generating = task.isGenerating();
      boolean processing = task.isProcessing();
      boolean retryOnFail = task.isRetryOnFail();
      boolean replaceMessage = task.isReplaceMessage();
      boolean trackIntegrity = task.isTrackIntegrity();
      String svcType = task.getSvcType();

      String idEnc = StringEncoderDecoder.encode(id);
      String idHex = StringEncoderDecoder.encodeHEX(id);
      String nameEnc = StringEncoderDecoder.encode(name);
      String providerEnc = StringEncoderDecoder.encode(provider);
      String svcTypeEnc = StringEncoderDecoder.encode(svcType);

      %><tr class=row<%=rowN++&1%>>
        <td><input class=check type=checkbox name=checked id=checked<%=idHex%> value="<%=idEnc%>" <%=bean.isTaskChecked(id) ? "checked" : ""%> onclick="checkTasks();"></td>
        <td><label for=checked<%=idHex%>><%=nameEnc%></label></td>
        <td nowrap><%=providerEnc%></td>
        <td><%if (enabled       ){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td><span datasrc=#tdcTasksStatuses DATAFORMATAS=html datafld="gen<%=idHex%>"><%if (generating    ){%><img src="/images/ic_running.gif"><%}else{%><img src="/images/ic_stopped.gif"><%}%></span></td>
        <td><span datasrc=#tdcTasksStatuses DATAFORMATAS=html datafld="prc<%=idHex%>"><%if (processing    ){%><img src="/images/ic_running.gif"><%}else{%><img src="/images/ic_stopped.gif"><%}%></span></td>
        <td><%=priority%></td>
        <td><%if (retryOnFail   ){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td><%if (replaceMessage){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td><%if (trackIntegrity){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
        <td nowrap><%=svcTypeEnc%></td>
      </tr><%
    }
    %></table><%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%><%
  }%>
</div><%
  page_menu_begin(out);
  page_menu_button(out, "mbEnableTask",  "Enable",  "");
  page_menu_button(out, "mbDisableTask", "Disable", "");
  page_menu_button(out, "mbStartTask",   "Start", "Start messages generation");
  page_menu_button(out, "mbStopTask",    "Stop",  "Stop messages generation");
  page_menu_space(out);
  page_menu_end(out);
%>
<script>
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
  mbEnableTask.disabled = mbDisableTask.disabled = mbStartTask.disabled = mbStopTask.disabled = disabledElem;
  return true;
}
checkTasks();
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>