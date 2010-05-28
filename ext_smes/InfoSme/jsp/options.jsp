<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper" %>
<%@ page import="java.util.Iterator"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Options" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
    bean.getSmscHelper().processRequest(request);
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/time.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0>
<col width="1%">
<col width="99%">
<% int rowN = 0;%>
<tr class=row<%=rowN++&1%>>
  <th>Address</th>
  <td><input class=txt name=address value="<%=StringEncoderDecoder.encode(bean.getAddress())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Service type</th>
  <td><input class=txt name=svcType value="<%=StringEncoderDecoder.encode(bean.getSvcType())%>" maxlength="5"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Protocol ID</th>
  <td><input class=txt name=protocolId value="<%=StringEncoderDecoder.encode(bean.getProtocolId())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Max messages per second</th>
  <td><input class=txt name=maxMessagesPerSecond value="<%=StringEncoderDecoder.encode(bean.getMaxMessagesPerSecond())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Unresponded messages max</th>
  <td><input class=txt name=unrespondedMessagesMax value="<%=StringEncoderDecoder.encode(bean.getUnrespondedMessagesMax())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Unresponded messages sleep</th>
  <td><input class=txt name=unrespondedMessagesSleep value="<%=StringEncoderDecoder.encode(bean.getUnrespondedMessagesSleep())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Response wait time</th>
  <td><input class=txt name=responceWaitTime value="<%=StringEncoderDecoder.encode(bean.getResponceWaitTime())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Receipt wait time</th>
  <td><input class=txt name=receiptWaitTime value="<%=StringEncoderDecoder.encode(bean.getReceiptWaitTime())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks task tables prefix</th>
  <td><input class=txt name=tasksTaskTablesPrefix value="<%=StringEncoderDecoder.encode(bean.getTasksTaskTablesPrefix())%>" maxlength="10"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks switch timeout</th>
  <td><input class=txt name=tasksSwitchTimeout value="<%=StringEncoderDecoder.encode(bean.getTasksSwitchTimeout())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks thread pool max</th>
  <td><input class=txt name=tasksThreadPoolMax value="<%=StringEncoderDecoder.encode(bean.getTasksThreadPoolMax())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks thread pool init</th>
  <td><input class=txt name=tasksThreadPoolInit value="<%=StringEncoderDecoder.encode(bean.getTasksThreadPoolInit())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Events thread pool max</th>
  <td><input class=txt name=eventsThreadPoolMax value="<%=StringEncoderDecoder.encode(bean.getEventsThreadPoolMax())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Events thread pool init</th>
  <td><input class=txt name=eventsThreadPoolInit value="<%=StringEncoderDecoder.encode(bean.getEventsThreadPoolInit())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Admin host</th>
  <td><input class=txt name=adminHost value="<%=StringEncoderDecoder.encode(bean.getAdminHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Admin port</th>
  <td><input class=txt name=adminPort value="<%=StringEncoderDecoder.encode(bean.getAdminPort())%>"></td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th>Store location</th>
  <td><input class=txt name=storeLocation value="<%=StringEncoderDecoder.encode(bean.getStoreLocation())%>"></td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th>Archive location</th>
  <td><input class=txt name=archiveLocation value="<%=StringEncoderDecoder.encode(bean.getArchiveLocation())%>"></td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th>Statistics store location</th>
  <td><input class=txt name=statStoreLocation value="<%=StringEncoderDecoder.encode(bean.getStatStoreLocation())%>"></td>
</tr>

<tr class=row<%=rowN++&1%>>
    <td colspan="2">
        <%  final DynamicTableHelper tableHelper = bean.getSmscHelper();%>
          <%@ include file="/WEB-INF/inc/dynamic_table.jsp"%>
    </td>
</tr>
    
<tr class=row<%=rowN++&1%>>
  <th>Default smsc connector</th>
  <td><input class=txt name=defSmscConn value="<%=StringEncoderDecoder.encode(bean.getDefSmscConn())%>"></td>
</tr>

</table>
<br/>
<%startSection(out, "siebel", "infosme.title.siebel", false);  %>
<table class=properties_list cellspacing=0>
<col width="1%">
<col width="99%">
<%rowN = 0;%>

<tr class=row<%=rowN++&1%>>
  <td colspan="2"><div class="page_subtitle"><%= getLocString("infosme.title.siebel.tm")%></div></td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><label for=siebelTMPeriod><%= getLocString("infosme.label.siebelTMPeriod")%></label></th>
  <td>
    <input class=txt id=siebelTMPeriod name=siebelTMPeriod value="<%=StringEncoderDecoder.encode(bean.getSiebelTMPeriod())%>" validation="positive">
  </td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><label for=siebelTMRemoveStopped><%= getLocString("infosme.label.siebelTMRemoveStopped")%></label></th>
  <td>
    <input class=check type=checkbox id=siebelTMRemoveStopped name=siebelTMRemoveStopped <%=bean.isSiebelTMRemoveStopped() ? "checked" : ""%>>
     </td>
</tr>

<tr class=row<%=rowN++&1%>>
  <td colspan="2"><div class="page_subtitle"><%= getLocString("infosme.title.siebel.dp")%></div></td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><label for=siebelJDBCDriver><%= getLocString("infosme.label.siebelJDBCDriver")%></label></th>
  <td>
    <input class=txt id=siebelJDBCDriver name=siebelJDBCDriver value="<%=StringEncoderDecoder.encode(bean.getSiebelJDBCDriver())%>" validation="nonEmpty">
  </td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><label for=siebelJDBCSource><%= getLocString("infosme.label.siebelJDBCSource")%></label></th>
  <td>
    <input class=txt id=siebelJDBCSource name=siebelJDBCSource value="<%=StringEncoderDecoder.encode(bean.getSiebelJDBCSource())%>" validation="nonEmpty">
  </td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><label for=siebelJDBCUser><%= getLocString("infosme.label.siebelJDBCUser")%></label></th>
  <td>
    <input class=txt id=siebelJDBCUser name=siebelJDBCUser value="<%=StringEncoderDecoder.encode(bean.getSiebelJDBCUser())%>">
  </td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><label for=siebelJDBCUser><%= getLocString("infosme.label.siebelJDBCPass")%></label></th>
  <td>
    <input class=txt id=siebelJDBCPass name=siebelJDBCPass value="<%=StringEncoderDecoder.encode(bean.getSiebelJDBCPass())%>">
  </td>
</tr>

<tr class=row<%=rowN++&1%>>
  <td colspan="2"><div class="page_subtitle"><%= getLocString("infosme.title.siebel.task")%></div></td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><label for=siebelTransactionMode><%= getLocString("infosme.label.transaction_mode")%></label></th>
  <td>
    <input class=check type=checkbox id=siebelTransactionMode name=siebelTransactionMode <%=bean.isSiebelTransactionMode() ? "checked" : ""%>>
  </td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.active_period")%></th>
  <td>
    <input class=timeField id=siebelActivePeriodStart name=siebelActivePeriodStart value="<%=StringEncoderDecoder.encode(bean.getSiebelActivePeriodStart())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(siebelActivePeriodStart, false, true);">...</button>
    &nbsp;<%= getLocString("infosme.label.active_period_to")%>&nbsp;
    <input class=timeField id=siebelActivePeriodEnd name=siebelActivePeriodEnd value="<%=StringEncoderDecoder.encode(bean.getSiebelActivePeriodEnd())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(siebelActivePeriodEnd, false, true);">...</button>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.active_weekdays")%></th>
  <td>
    <table>
    <col width="1%"><col width="32%">
    <col width="1%"><col width="32%">
    <col width="1%"><col width="32%">
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=siebelActiveWeekDays id=activeWeekDays_Mon value=Mon <%=bean.isWeekDayActive("Mon") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Mon><%= getLocString("infosme.label.monday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=siebelActiveWeekDays id=activeWeekDays_Thu value=Thu <%=bean.isWeekDayActive("Thu") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Thu><%= getLocString("infosme.label.thursday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=siebelActiveWeekDays id=activeWeekDays_Sat value=Sat <%=bean.isWeekDayActive("Sat") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Sat><%= getLocString("infosme.label.saturday")%></label></td>
    <tr>
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=siebelActiveWeekDays id=activeWeekDays_Tue value=Tue <%=bean.isWeekDayActive("Tue") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Tue><%= getLocString("infosme.label.tuesday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=siebelActiveWeekDays id=activeWeekDays_Fri value=Fri <%=bean.isWeekDayActive("Fri") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Fri><%= getLocString("infosme.label.friday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=siebelActiveWeekDays id=activeWeekDays_Sun value=Sun <%=bean.isWeekDayActive("Sun") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Sun><%= getLocString("infosme.label.sunday")%></label></td>
    <tr>
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=siebelActiveWeekDays id=activeWeekDays_Wed value=Wed <%=bean.isWeekDayActive("Wed") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Wed><%= getLocString("infosme.label.wednesday")%></label></td>
    <tr>
    </table>
  </td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><label for=siebelRetryOnFail><%= getLocString("infosme.label.retry_policy")%></label></th>
  <td>
    <input class=check type=checkbox id=siebelRetryOnFail name=siebelRetryOnFail <%=bean.isSiebelRetryOnFail() ? "checked" : ""%>>
    <select id="siebelRetryPolicy" name="siebelRetryPolicy">
          <%for (Iterator iter = bean.getRetryPolicies().iterator(); iter.hasNext();) {
            String policy = (String)iter.next();
          %>
          <option value="<%=policy%>" <%=bean.getSiebelRetryPolicy() != null && bean.getSiebelRetryPolicy().equals(policy) ? "SELECTED" : ""%>><%=StringEncoderDecoder.encode(policy)%></option>
          <%}%>
        </select>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=siebelReplaceMessage><%= getLocString("infosme.label.replace_messages")%></label></th>
  <td>
    <input class=check type=checkbox id=siebelReplaceMessage name=siebelReplaceMessage <%=bean.isSiebelReplaceMessage() ? "checked" : ""%> onClick="document.getElementById('siebelSvcType').disabled = !this.checked;">
    <input class=txt id=siebelSvcType name=siebelSvcType value="<%=StringEncoderDecoder.encode(bean.getSiebelSvcType())%>" maxlength=5 validation="id" onkeyup="resetValidation(this)">
    <script>document.getElementById('siebelSvcType').disabled = !document.getElementById('siebelReplaceMessage').checked;</script>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=siebelTrackIntegrity><%= getLocString("infosme.label.integrity")%></label></th>
  <td><input class=check type=checkbox id=siebelTrackIntegrity name=siebelTrackIntegrity <%=bean.isSiebelTrackIntegrity() ? "checked" : ""%>>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=siebelKeepHistory><%= getLocString("infosme.label.keep_history")%></label></th>
  <td><input class=check type=checkbox id=siebelKeepHistory name=siebelKeepHistory <%=bean.isSiebelKeepHistory() ? "checked" : ""%>>
  </td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("infosme.label.cacheSize")%></th>
  <td><input class=txt name=siebelMessagesCacheSize value="<%=StringEncoderDecoder.encode(bean.getSiebelMessagesCacheSize())%>">
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("infosme.label.cacheSleep")%></th>
  <td>
     <input class=txt name=siebelMessagesCacheSleep value="<%=StringEncoderDecoder.encode(bean.getSiebelMessagesCacheSleep())%>">secs
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("infosme.label.uncommitedGeneration")%></th>
  <td><input class=txt name=siebelUncommitedInGeneration value="<%=StringEncoderDecoder.encode(bean.getSiebelUncommitedInGeneration())%>">
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("infosme.label.uncommitedProcess")%></th>
  <td>
      <input class=txt name=siebelUncommitedInProcess value="<%=StringEncoderDecoder.encode(bean.getSiebelUncommitedInProcess())%>">
  </td>
</tr>


</table>

<%   finishSection(out); %>

</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",  "infosme.hint.done_editing");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "infosme.hint.cancel_changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>