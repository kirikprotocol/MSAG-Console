<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants, ru.novosoft.smsc.mcisme.beans.Index,
                 ru.novosoft.smsc.jsp.SMSCJspException, ru.novosoft.smsc.jsp.SMSCErrors,
                 java.util.*, ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.util.Functions,
                 ru.novosoft.smsc.admin.service.ServiceInfo,
                 ru.novosoft.smsc.mcisme.backend.CountersSet"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	TITLE="Missed Calls Info SME Administration";
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
</OBJECT><script>
function refreshMCISmeTdcStatuses()
{
	document.all.tdcMCISmeStatuses.DataURL = document.all.tdcMCISmeStatuses.DataURL;
	document.all.tdcMCISmeStatuses.reset();
	window.setTimeout(refreshMCISmeTdcStatuses, 2000);
}
refreshMCISmeTdcStatuses();
</script>
<div class=content>
<script>
function checkApplyResetButtons()
{
  opForm.all.mbApply.disabled = !(opForm.all.allCheck.checked);
  opForm.all.mbReset.disabled = !((<%=bean.isConfigChanged()%> && opForm.all.allCheck.checked));
}
</script>
<%! private String status(boolean isChanged) { // TODO: ???
  return ((isChanged) ? "<span style='color:red;'>changed</span>":
                        "<span style='color:green;'>clear</span>");
}%>
<input type=hidden name=edit>
<div class=page_subtitle>Configuration status</div>
<table class=properties_list cellspacing=0>
  <tr>
    <th width="20%" nowrap valign=top>General configuration</th>
    <td width="80%" nowrap valign=top><%= status(bean.isOptionsChanged())%></td>
  </tr>
  <tr>
    <th width="20%" nowrap valign=top>Drivers configuration</th>
    <td width="80%" nowrap valign=top><%= status(bean.isDriversChanged())%></td>
  </tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply changes", bean.isConfigChanged());
page_menu_button(out, "mbReset",  "Reset",  "Discard changes", bean.isConfigChanged());
page_menu_space(out);
page_menu_button(out, "mbStart", "Start", "Start MCI Sme service", bean.getStatus() == ServiceInfo.STATUS_STOPPED);
page_menu_button(out, "mbStop",  "Stop",  "Stop MCI Sme service", bean.getStatus() == ServiceInfo.STATUS_RUNNING);
page_menu_end(out);
%>
<script language="JavaScript">
function refreshStartStopButtonsStatus()
{
	document.all.mbStart.disabled = (document.all.RUNNING_STATUSERVICE_MCISme.innerText != "stopped");
	document.all.mbStop.disabled = (document.all.RUNNING_STATUSERVICE_MCISme.innerText != "running");
	window.setTimeout(refreshStartStopButtonsStatus, 500);
}
refreshStartStopButtonsStatus();
</script><%
  CountersSet statSet = bean.getStatistics();
%><div class=content>
<div class=page_subtitle>Service runtime status</div>
<table class=properties_list cellspacing=0>
  <tr class=row1>
    <th width="20%" nowrap valign=top>&nbsp;</th>
    <td width="80%" nowrap valign=top>&nbsp;</td>
  </tr>
  <tr class=row0>
    <th width="20%" nowrap valign=top>Abonents in service:&nbsp;</th>
    <td width="80%" nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="MCISmeActiveAbonents"><%= bean.getActiveTasksCount()%></span></td>
  </tr>
</table><br/>
<table class=properties_list cellspacing=0>
  <tr class=row1>
    <th width="20%" nowrap valign=top>&nbsp;</th>
    <td width="20%" nowrap valign=top>Missed calls</td>
    <td width="20%" nowrap valign=top>Abonents informed</td>
    <td width="20%" nowrap valign=top>Failures count</td>
    <td width="20%" nowrap valign=top>Notifications sent</td>
  </tr>
  <tr class=row0>
    <th width="20%" nowrap valign=top>Last hour statistics:&nbsp;</th>
    <td width="20%" nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="statMissed">   <%= (statSet != null) ? statSet.missed:0%>   </span></td>
    <td width="20%" nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="statDelivered"><%= (statSet != null) ? statSet.delivered:0%></span></td>
    <td width="20%" nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="statFailed">   <%= (statSet != null) ? statSet.failed:0%>   </span></td>
    <td width="20%" nowrap valign=top><span datasrc=#tdcMCISmeStatuses DATAFORMATAS=html datafld="statNotified"> <%= (statSet != null) ? statSet.notified:0%> </span></td>
  </tr>
</table>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>