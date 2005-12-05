<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.util.Functions,
                 ru.novosoft.smsc.mcisme.backend.CountersSet,
                 ru.novosoft.smsc.mcisme.backend.RuntimeSet"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
    ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	TITLE=getLocString("mcisme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
    int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<OBJECT id="tdcMCISmeStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="/smsc/smsc/esme_MCISme/statuses.jsp">
	<PARAM NAME="UseHeader" VALUE="True">
	<PARAM NAME="TextQualifier" VALUE='"'>
</OBJECT><script type="text/javascript">
function refreshMCISmeTdcStatuses()
{
	document.all.tdcMCISmeStatuses.DataURL = document.all.tdcMCISmeStatuses.DataURL;
	document.all.tdcMCISmeStatuses.reset();
	window.setTimeout(refreshMCISmeTdcStatuses, 2000);
}
refreshMCISmeTdcStatuses();
</script>
<div class=content>
<script type="text/javascript">
function checkApplyResetButtons()
{
  opForm.all.mbApply.disabled = !(opForm.all.allCheck.checked);
  opForm.all.mbReset.disabled = !((<%=bean.isConfigChanged()%> && opForm.all.allCheck.checked));
}
</script>
<%! private String configStatus(boolean isChanged) {
    return (isChanged) ? "<span style='color:red;'>changed</span>":"clear";
}%>
<!--%! private String runStatus(byte status) {
  if      (status == ServiceInfo.STATUS_RUNNING)  return "<span style='color:green;'>running</span>";
  else if (status == ServiceInfo.STATUS_STARTING) return "<span style='color:green;'>starting</span>";
  else if (status == ServiceInfo.STATUS_STOPPING) return "<span style='color:red;'  >stopping</span>";
  else if (status == ServiceInfo.STATUS_STOPPED)  return "stopped";
  return "<span style='color:blue;'>unknown</span>";
}%-->
<input type=hidden name=edit>

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
      <td style="padding-right:3px"><input class=check type=checkbox id=toStartService name=toStart value=service onClick="checkStartStop();"></td>
      <th><label for=toStartSme><%= getLocString("mcisme.label.mci_service")%></label></th>
      <td><%= serviceStatus(bean.getAppContext(), bean.getSmeId(),
                            "MCISme_RUNNING_STATUSERVICE_MCISme")%></td>
    </tr>
    <tr class=row1 id=profRow>
      <td style="padding-right:3px"><input class=check type=checkbox id=toStartProfiler name=toStart value=profiler onClick="checkStartStop();"></td>
      <th><label for=toStartProfiler><%= getLocString("mcisme.label.mci_profiler")%><label></th>
      <td><%= serviceStatus(bean.getAppContext(), bean.getProfilerSmeId(),
                            "MCIProf_RUNNING_STATUSERVICE_MCIProf")%></td>
    </tr>
    <tr class=row0>
      <td style="padding-right:3px">&nbsp;</td><th>&nbsp;</th><td>&nbsp;</td>
    </tr>
    <tr class=row1>
      <td style="padding-right:3px">&nbsp;</td><th>&nbsp;</th><td>&nbsp;</td>
    </tr>
    </table>
  </td>
  <td>&nbsp;</td>
  <td>
    <table class=properties_list cellspacing=0>
    <col width="80%">
    <col width="20%" align="right">
    <tr class=row0>
      <th><%= getLocString("mcisme.label.get_settings")%></th>
      <td><%= configStatus(bean.isOptionsChanged())%></td>
    </tr>
    <tr class=row1>
      <th><%= getLocString("mcisme.label.ds_drivers_config")%></th>
      <td><%= configStatus(bean.isDriversChanged())%></td>
    </tr>
    <tr class=row0>
      <th><%= getLocString("mcisme.label.templates_config")%></th>
      <td><%= configStatus(bean.isTemplatesChanged())%></td>
    </tr>
    <tr class=row1>
      <th><%= getLocString("mcisme.label.rules_config")%></th>
      <td><%= configStatus(bean.isRulesChanged())%></td>
    </tr>
    <tr class=row0>
      <th><%= getLocString("mcisme.label.tz_offsets")%></th>
      <td><%= configStatus(bean.isOffsetsChanged())%></td>
    </tr>
    </table>
  </td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbStart", "common.buttons.start", "mcisme.hint.start_services");
page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "mcisme.hint.stop_services");
page_menu_space(out);
page_menu_button(session, out, "mbApply",  "common.buttons.apply",  "mcisme.hint.apply", bean.isConfigChanged());
page_menu_button(session, out, "mbReset",  "common.buttons.reset",  "mcisme.hint.reset", bean.isConfigChanged());
page_menu_end(out);
%>
<script type="text/javascript" language="JavaScript">
function checkStartStop()
{
  var serviceChecked  = opForm.all.toStartService.checked;
  var serviceRunning  = document.all.MCISme_RUNNING_STATUSERVICE_MCISme.innerText == "running";
  var serviceStopped  = document.all.MCISme_RUNNING_STATUSERVICE_MCISme.innerText == "stopped";
  var profilerChecked = opForm.all.toStartProfiler.checked;
  var profilerRunning = document.all.MCIProf_RUNNING_STATUSERVICE_MCIProf.innerText == "running";
  var profilerStopped = document.all.MCIProf_RUNNING_STATUSERVICE_MCIProf.innerText == "stopped";

  var serviceStart  = serviceStopped && serviceChecked;
  var serviceStop   = serviceRunning && serviceChecked;
  var profilerStart = profilerStopped && profilerChecked;
  var profilerStop  = profilerRunning && profilerChecked;

  var start = serviceStart || profilerStart;
  var stop  = serviceStop  || profilerStop;

  opForm.all.mbStart.disabled = !start;
  opForm.all.mbStop.disabled = !stop;

  window.setTimeout(checkStartStop, 500);
}
checkStartStop();
</script><%
  CountersSet cstatSet = bean.getStatistics(); RuntimeSet  rstatSet = bean.getRuntime();
%><div class=content>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.service_runtime")%></div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%" align=left>
<col width="25%" align=left>
<col width="30%" align=left>
<col width="25%" align=left>
<thread>
  <tr class=row1>
    <th><%= getLocString("mcisme.label.in_service")%></th>
    <th><%= getLocString("mcisme.label.io_queues")%></th>
    <th><%= getLocString("mcisme.label.io_speed")%></th>
    <th>&nbsp;</th>
  </tr>
  <tr class=row0>
    <td nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="rstatActive"><%= (rstatSet != null) ? rstatSet.activeTasks:0%></span></td>
    <td nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="rstatInQS"><%= (rstatSet != null) ? rstatSet.inQueueSize:0%></span>/<span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="rstatOutQS"><%= (rstatSet != null) ? rstatSet.outQueueSize:0%></span></td>
    <td nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="rstatInSpeed"><%= (rstatSet != null) ? rstatSet.inSpeed:0%></span>/<span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="rstatOutSpeed"><%= (rstatSet != null) ? rstatSet.outSpeed:0%></span></td>
    <td nowrap valign=top>&nbsp;</td>
  </tr>
</thread>
</table><br/>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.lh_stat")%></div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%" align=left>
<col width="25%" align=left>
<col width="30%" align=left>
<col width="25%" align=left>
<thread>
  <tr class=row1>
    <th><%= getLocString("mcisme.label.missed_calls")%></th>
    <th><%= getLocString("mcisme.label.abonents_informed")%></th>
    <th><%= getLocString("mcisme.label.failures_count")%></th>
    <th><%= getLocString("mcisme.label.notifications_sent")%></th>
  </tr>
  <tr class=row0>
    <td nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="cstatMissed"><%= (cstatSet != null) ? cstatSet.missed:0%></span></td>
    <td nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="cstatDelivered"><%= (cstatSet != null) ? cstatSet.delivered:0%></span></td>
    <td nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="cstatFailed"><%= (cstatSet != null) ? cstatSet.failed:0%></span></td>
    <td nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="cstatNotified"><%= (cstatSet != null) ? cstatSet.notified:0%></span></td>
  </tr>
</thread>
</table>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>