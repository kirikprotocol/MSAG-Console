<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.TaskEdit,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.Iterator, ru.novosoft.smsc.infosme.backend.config.tasks.Task"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.TaskEdit" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";

	int beanResult = bean.process(request);
	switch(beanResult) {
		case TaskEdit.RESULT_DONE:
			response.sendRedirect("tasks.jsp");
			return;
    case TaskEdit.RESULT_MESSAGES:
      response.sendRedirect("messages.jsp?taskId=" + bean.getId() + "&initialized=true");
      return;
    case TaskEdit.RESULT_STATISTICS:
      response.sendRedirect("stat.jsp?taskId=" + bean.getId() +"&fromDate="+(bean.getStartDate() == null ? "" : bean.getStartDate())
          + "&tillDate="+(bean.getEndDate() == null ? "" : bean.getEndDate()) + "&initialized=true");
      return;
		default:{%><%@ include file="inc/menu_switch.jsp"%><%}
	}
  int rowN = 0;
  boolean admin = InfoSmeBean.isUserAdmin(request);
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="/WEB-INF/inc/time.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=create value=<%=bean.isCreate()%>>
<input type=hidden name=delivery value=<%=bean.isDelivery()%>>
<input type=hidden name=oldTask value="<%=bean.getOldTask()%>">
<input type=hidden name=oldTaskName value="<%=bean.getOldTaskName()%>">
<table class=properties_list>
<col width="10%">
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.task_id")%></th>
  <td><input class=txt id=id name=id readonly value="<%=StringEncoderDecoder.encode(bean.getId())%>"></td>
</tr>
<% if (admin) { %>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.owner")%></th>
  <td><select id="owner" name="owner">
    <% Collection users = bean.getUsers();
       if (!users.contains(bean.getOwner())) {%>
    <option value="<%=bean.getOwner()%>" SELECTED><%=bean.getOwner()%> (unavailable)</option>
    <% } %>
    <%for (Iterator iter = users.iterator(); iter.hasNext();) {
      String user = (String)iter.next();%>
    <option value="<%=user%>" <%=bean.getOwner() != null && bean.getOwner().equals(user) ? "SELECTED" : ""%>><%=user%></option>
    <% } %>
  </select>    
</tr>
<% } %>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.task_name")%></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt id=name name=name value="<%=StringEncoderDecoder.encode(bean.getName())%>" validation="nonEmpty"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getName())%><%
    }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.originating_address")%></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt id=address name=address value="<%=StringEncoderDecoder.encode(bean.getAddress())%>"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getAddress())%><%
  }%>
  </td>
</tr>
<% if (admin) { %>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.provider")%></th>
  <td><%
    if (bean.isDelivery()) {
      %><input type=hidden name=provider value=<%= Task.INFOSME_EXT_PROVIDER%>><%= Task.INFOSME_EXT_PROVIDER%><%
    } else if (bean.isSmeRunning()) {
      %><select name=provider><%
      for (Iterator i = bean.getAllProviders().iterator(); i.hasNext();) {
        String providerName = (String) i.next();
        String providerNameEnc = StringEncoderDecoder.encode(providerName);
        %><option value="<%=providerNameEnc%>"<%=providerName.equals(bean.getProvider()) ? " selected" : ""%>><%=providerNameEnc%></option><%
      }%></select><%
    } else {
      %><%=StringEncoderDecoder.encode(bean.getProvider())%><%
    }%>
  </td>
</tr>
<% } %>
<tr class=row<%=rowN++&1%>>
  <th><label for=enabled><%= getLocString("infosme.label.on")%></label></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=enabled name=enabled <%=bean.isEnabled() ? "checked" : ""%>><%
  } else {
    %><%=bean.isEnabled()%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=transactionMode><%= getLocString("infosme.label.transaction_mode")%></label></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=transactionMode name=transactionMode <%=bean.isTransactionMode() ? "checked" : ""%>><%
  } else {
    %><%=bean.isTransactionMode() ? "enabled" : "disabled"%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=useDataSm><%= getLocString("infosme.label.use_data_sm")%></label></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=useDataSm name=useDataSm <%=bean.isUseDataSm() ? "checked" : ""%>><%
  } else {
    %><%=bean.isUseDataSm() ? "enabled" : "disabled"%><%
  }%>
  </td>
</tr>
<%if ( bean.isUssdPushAllowed() ) {%>
<tr class=row<%=rowN++&1%>>
  <th><label for=useUssdPush><%= getLocString("infosme.label.use_ussd_push")%></label></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=useUssdPush name=useUssdPush <%=bean.isUseUssdPush() ? "checked" : ""%>><%
  } else {
    %><%=bean.isUseUssdPush() ? "enabled" : "disabled"%><%
  }%>
  </td>
</tr>
<% } %>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.priority")%></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=priority value="<%=StringEncoderDecoder.encode(bean.getPriority())%>" validation="int_range" range_min="1" range_max="100" onkeyup="resetValidation(this)"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getPriority())%><%
  }%>
  </td>
</tr>

<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.active_period")%></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=timeField id=activePeriodStart name=activePeriodStart value="<%=StringEncoderDecoder.encode(bean.getActivePeriodStart())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodStart, false, true);">...</button>
    &nbsp;<%= getLocString("infosme.label.active_period_to")%>&nbsp;
    <input class=timeField id=activePeriodEnd name=activePeriodEnd value="<%=StringEncoderDecoder.encode(bean.getActivePeriodEnd())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodEnd, false, true);">...</button><%
  } else {
    String from = (bean.getActivePeriodStart() != null && bean.getActivePeriodStart().trim().length() > 0) ?
                   StringEncoderDecoder.encode(bean.getActivePeriodStart()) : "-";
    %>from <%=from%>&nbsp;<%
    String till = (bean.getActivePeriodEnd() != null && bean.getActivePeriodEnd().trim().length() > 0) ?
                 StringEncoderDecoder.encode(bean.getActivePeriodEnd()) : "-";
    %>till <%=till%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.active_weekdays")%></th>
  <td><%if (bean.isSmeRunning()) {
    %><table>
    <col width="1%"><col width="32%">
    <col width="1%"><col width="32%">
    <col width="1%"><col width="32%">
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Mon value=Mon <%=bean.isWeekDayActive("Mon") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Mon><%= getLocString("infosme.label.monday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Thu value=Thu <%=bean.isWeekDayActive("Thu") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Thu><%= getLocString("infosme.label.thursday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Sat value=Sat <%=bean.isWeekDayActive("Sat") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Sat><%= getLocString("infosme.label.saturday")%></label></td>
    <tr>
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Tue value=Tue <%=bean.isWeekDayActive("Tue") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Tue><%= getLocString("infosme.label.tuesday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Fri value=Fri <%=bean.isWeekDayActive("Fri") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Fri><%= getLocString("infosme.label.friday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Sun value=Sun <%=bean.isWeekDayActive("Sun") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Sun><%= getLocString("infosme.label.sunday")%></label></td>
    <tr>
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Wed value=Wed <%=bean.isWeekDayActive("Wed") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Wed><%= getLocString("infosme.label.wednesday")%></label></td>
    <tr>
    </table><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getActiveWeekDaysString())%><%
  }%>
  </td>
</tr>
<% if (admin) { %>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.vperiod_date")%></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=timeField id=validityPeriod name=validityPeriod value="<%=StringEncoderDecoder.encode(bean.getValidityPeriod())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(validityPeriod, false, true);">...</button>
    &nbsp;<%= getLocString("infosme.label.vperiod_date_or")%>&nbsp;
    <input class=calendarField id=validityDate name=validityDate value="<%=StringEncoderDecoder.encode(bean.getValidityDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(validityDate, false, true);">...</button><%
  } else {
    if (bean.getValidityPeriod() != null && bean.getValidityPeriod().trim().length() > 0) {
      %><%=StringEncoderDecoder.encode(bean.getValidityPeriod())%><%
    } else if (bean.getValidityDate() != null && bean.getValidityDate().trim().length() > 0) {
      %><%=StringEncoderDecoder.encode(bean.getValidityDate())%><%
    } else {
      %>not specified<%
    }
  }%>
  </td>
</tr>
<% } %>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.start_date")%></th>
  <% if (bean.isCreate()) {%>
  <td nowrap><input class=calendarField id=startDate name=startDate value="<%=bean.getStartDate()%>"><button class=calendarButton type=button onclick="return showCalendar(startDate, false, true);">...</button></td>
  <% } else {%>
  <td nowrap><input class=calendarField id=startDate name=startDate value="<%=bean.getStartDate()%>" readonly></td>
  <% } %>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.end_date")%></th>
  <td nowrap><%if (bean.isSmeRunning()) {
    %><input class=calendarField id=endDate name=endDate value="<%=StringEncoderDecoder.encode(bean.getEndDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(endDate, false, true);">...</button><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getEndDate())%><%
  }%>
  </td>
</tr>
<%if (!bean.isDelivery()) {%>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.query")%></th>
  <td><%if (bean.isSmeRunning()) {
    %><textarea name=query><%=StringEncoderDecoder.encode(bean.getQuery())%></textarea><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getQuery())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.template")%></th>
  <td><%if (bean.isSmeRunning()) {
    %><textarea name=template><%=StringEncoderDecoder.encode(bean.getTemplate())%></textarea><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getTemplate())%><%
  }%>
  </td>
</tr>
<%if (bean.isSmeRunning()) {%>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.transliterate_text")%></th>
  <td><input class=check type=checkbox id=transliterate name=transliterate <%=bean.isTransliterate() ? "checked" : ""%>></td>
</tr>
<%}}%>
<% if (admin) { %>
<tr class=row<%=rowN++&1%>>
  <th><label for=retryOnFail><%= getLocString("infosme.label.retry_policy")%></label></th>
  <td><%if (bean.isSmeRunning()) {%>
    <input class=check type=checkbox id=retryOnFail name=retryOnFail <%=bean.isRetryOnFail() ? "checked" : ""%>>
    <select id="retryPolicy" name="retryPolicy">
          <%for (Iterator iter = bean.getRetryPolicies().iterator(); iter.hasNext();) {
            String policy = (String)iter.next();
          %>
          <option value="<%=policy%>" <%=bean.getRetryPolicy() != null && bean.getRetryPolicy().equals(policy) ? "SELECTED" : ""%>><%=StringEncoderDecoder.encode(policy)%></option>
          <%}%>
        </select>
    <%
  } else {
    if (bean.isRetryOnFail() && bean.getRetryPolicy() != null && bean.getRetryPolicy().trim().length() > 0) {
      %>enabled, retry policy is '<%=StringEncoderDecoder.encode(bean.getRetryPolicy())%>'<%
    } else {
      %>disabled<%
    }
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=replaceMessage><%= getLocString("infosme.label.replace_messages")%></label></th>
  <td><%if (bean.isSmeRunning()) {%>
    <input class=check type=checkbox id=replaceMessage name=replaceMessage <%=bean.isReplaceMessage() ? "checked" : ""%> onClick="document.getElementById('svcType').disabled = !this.checked;">
    <input class=txt id=svcType name=svcType value="<%=StringEncoderDecoder.encode(bean.getSvcType())%>" maxlength=5 validation="id" onkeyup="resetValidation(this)">
    <script>document.getElementById('svcType').disabled = !document.getElementById('replaceMessage').checked;</script><%
  } else {
    if (bean.isReplaceMessage() && bean.getSvcType() != null && bean.getSvcType().trim().length() > 0) {
      %>enabled, "<%=StringEncoderDecoder.encode(bean.getSvcType())%>"<%
    } else {
      %>disabled<%
    }
  }%>
  </td>
</tr>
<%if (!bean.isDelivery()) {%>
<tr class=row<%=rowN++&1%>>
  <th><label for=trackIntegrity><%= getLocString("infosme.label.integrity")%></label></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=trackIntegrity name=trackIntegrity <%=bean.isTrackIntegrity() ? "checked" : ""%>><%
  } else {
    %><%=bean.isTrackIntegrity() ? "enabled" : "disabled"%><%
  }%>
  </td>
</tr>
<%}%>
<tr class=row<%=rowN++&1%>>
  <th><label for=keepHistory><%= getLocString("infosme.label.keep_history")%></label></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=keepHistory name=keepHistory <%=bean.isKeepHistory() ? "checked" : ""%>><%
  } else {
    %><%=bean.isKeepHistory() ? "enabled" : "disabled"%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>System data source timeout</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=dsOwnTimeout value="<%=StringEncoderDecoder.encode(bean.getDsTimeout())%>">secs<%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getDsTimeout())%> secs<%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Messages cache size </th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=messagesCacheSize value="<%=StringEncoderDecoder.encode(bean.getMessagesCacheSize())%>"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getMessagesCacheSize())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Messages cache sleep</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=messagesCacheSleep value="<%=StringEncoderDecoder.encode(bean.getMessagesCacheSleep())%>">secs<%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getMessagesCacheSleep())%> secs<%
  }%>
  </td>
</tr>
<%if (!bean.isDelivery()) {%>
<tr class=row<%=rowN++&1%>>
  <th>Uncommited in generation</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=uncommitedInGeneration value="<%=StringEncoderDecoder.encode(bean.getUncommitedInGeneration())%>"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getUncommitedInGeneration())%><%
  }%>
  </td>
</tr>
<%}%>
<tr class=row<%=rowN++&1%>>
  <th>Uncommited in process</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=uncommitedInProcess value="<%=StringEncoderDecoder.encode(bean.getUncommitedInProcess())%>"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getUncommitedInProcess())%><%
  }%>
  </td>
</tr>
<% } %>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.flash")%></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=flash name=flash <%=bean.isFlash() ? "checked" : ""%>><%
  } else {
    %><%=bean.isFlash() ? "enabled" : "disabled"%><%
    }%>
  </td>
</tr>
<% if (bean.isDelivery() && !bean.isCreate() && bean.isSecret()) { %>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.secret_message")%></th>
  <td><%=StringEncoderDecoder.encode(bean.getSecretMessage())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.secret_flash")%></th>
  <td><%=bean.isSecretFlash() ? "enabled" : "disabled"%></td>
</tr>

<% } %>
</table>
</div><%
page_menu_begin(out);
if (bean.isSmeRunning()) {
  page_menu_button(session, out, "mbDone",   "common.buttons.done",  "infosme.hint.done_editing");
}
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "infosme.hint.cancel_changes", "clickCancel()");
page_menu_space(out);
page_menu_button(session, out, "mbMessages",   "infosme.menu.messages",  "");
page_menu_button(session, out, "mbStatistics",   "infosme.menu.statistics",  "");
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
