<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.TaskEdit,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.Iterator"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.TaskEdit" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE="Informer SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.process(request);
	switch(beanResult)
	{
		case TaskEdit.RESULT_DONE:
			response.sendRedirect("tasks.jsp");
			return;
		default:
      {
        %><%@ include file="inc/menu_switch.jsp"%><%
      }
	}
  int rowN = 0;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/time.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=create value=<%=bean.isCreate()%>>
<input type=hidden name=oldTask value="<%=bean.getOldTask()%>">
<table class=properties_list>
<col width="10%">
<tr class=row<%=rowN++&1%>>
  <th>Task ID</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=id value="<%=StringEncoderDecoder.encode(bean.getId())%>" maxlength="10" validation="id" onkeyup="resetValidation(this)"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getId())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Task name</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=name value="<%=StringEncoderDecoder.encode(bean.getName())%>"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getName())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Provider</th>
  <td><%if (bean.isSmeRunning()) {
    %><select name=provider><%
      for (Iterator i = bean.getAllProviders().iterator(); i.hasNext();) {
        String providerName = (String) i.next();
        String providerNameEnc = StringEncoderDecoder.encode(providerName);
        %><option value="<%=providerNameEnc%>"<%=providerName.equals(bean.getProvider()) ? " selected" : ""%>><%=providerNameEnc%></option><%
      }
    %></select><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getProvider())%><%
  }%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=enabled>Enabled</label></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=enabled name=enabled value=true <%=bean.isEnabled() ? "checked" : ""%>><%
  } else {
    %><%=bean.isEnabled()%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=transactionMode>transaction mode</label></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=transactionMode name=transactionMode value=true <%=bean.isTransactionMode() ? "checked" : ""%>><%
  } else {
    %><%=bean.isTransactionMode() ? "enabled" : "disabled"%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Priority</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=priority value="<%=StringEncoderDecoder.encode(bean.getPriority())%>" validation="int_range" range_min="1" range_max="100" onkeyup="resetValidation(this)"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getPriority())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Active period</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=timeField id=activePeriodStart name=activePeriodStart value="<%=StringEncoderDecoder.encode(bean.getActivePeriodStart())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodStart, false, true);">...</button>
    to
    <input class=timeField id=activePeriodEnd name=activePeriodEnd value="<%=StringEncoderDecoder.encode(bean.getActivePeriodEnd())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodEnd, false, true);">...</button><%
  } else {
    if (bean.getActivePeriodStart() != null && bean.getActivePeriodStart().trim().length() > 0) {
      %>from <%=StringEncoderDecoder.encode(bean.getActivePeriodStart())%> <%
    } else if (bean.getActivePeriodEnd() != null && bean.getActivePeriodEnd().trim().length() > 0) {
      %>to <%=StringEncoderDecoder.encode(bean.getActivePeriodEnd())%><%
    } else {
      %>not specified<%
    }
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Active weekdays</th>
  <td><%if (bean.isSmeRunning()) {
    %><table>
    <col width="1%"><col width="32%">
    <col width="1%"><col width="32%">
    <col width="1%"><col width="32%">
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Mon value=Mon <%=bean.isWeekDayActive("Mon") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Mon>Monday</label></td>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Thu value=Thu <%=bean.isWeekDayActive("Thu") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Thu>Thursday</label></td>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Sat value=Sat <%=bean.isWeekDayActive("Sat") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Sat>Saturday</label></td>
    <tr>
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Tue value=Tue <%=bean.isWeekDayActive("Tue") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Tue>Tuesday</label></td>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Fri value=Fri <%=bean.isWeekDayActive("Fri") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Fri>Friday</label></td>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Sun value=Sun <%=bean.isWeekDayActive("Sun") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Sun>Sunday</label></td>
    <tr>
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Wed value=Wed <%=bean.isWeekDayActive("Wed") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Wed>Wednesday</label></td>
    <tr>
    </table><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getActiveWeekDaysString())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Validity period or date</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=timeField id=validityPeriod name=validityPeriod value="<%=StringEncoderDecoder.encode(bean.getValidityPeriod())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(validityPeriod, false, true);">...</button>
    or
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
<tr class=row<%=rowN++&1%>>
  <th>End date</th>
  <td nowrap><%if (bean.isSmeRunning()) {
    %><input class=calendarField id=endDate name=endDate value="<%=StringEncoderDecoder.encode(bean.getEndDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(endDate, false, true);">...</button><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getEndDate())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Query</th>
  <td><%if (bean.isSmeRunning()) {
    %><textarea name=query><%=StringEncoderDecoder.encode(bean.getQuery())%></textarea><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getQuery())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Template</th>
  <td><%if (bean.isSmeRunning()) {
    %><textarea name=template><%=StringEncoderDecoder.encode(bean.getTemplate())%></textarea><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getTemplate())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=retryOnFail>Retry on fail (time)</label></th>
  <td><%if (bean.isSmeRunning()) {%>
    <input class=check type=checkbox id=retryOnFail name=retryOnFail value=true <%=bean.isRetryOnFail() ? "checked" : ""%> onClick="opForm.all.retryTime.disabled = opForm.all.retryTimeButton.disabled = !this.checked;">
    <input class=timeField id=retryTime name=retryTime value="<%=StringEncoderDecoder.encode(bean.getRetryTime())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button id=retryTimeButton onclick="return showTime(retryTime, false, true);">...</button>
    <script>opForm.all.retryTime.disabled = opForm.all.retryTimeButton.disabled = !opForm.all.retryOnFail.checked;</script><%
  } else {
    if (bean.isRetryOnFail() && bean.getRetryTime() != null && bean.getRetryTime().trim().length() > 0) {
      %>enabled, retry on <%=StringEncoderDecoder.encode(bean.getRetryTime())%> secs<%
    } else {
      %>disabled<%
    }
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=replaceMessage>Replace message (service type)</label></th>
  <td><%if (bean.isSmeRunning()) {%>
    <input class=check type=checkbox id=replaceMessage name=replaceMessage value=true <%=bean.isReplaceMessage() ? "checked" : ""%> onClick="opForm.all.svcType.disabled = !this.checked;">
    <input class=txt id=svcType name=svcType value="<%=StringEncoderDecoder.encode(bean.getSvcType())%>" maxlength=5 validation="id" onkeyup="resetValidation(this)">
    <script>opForm.all.svcType.disabled = !opForm.all.replaceMessage.checked;</script><%
  } else {
    if (bean.isReplaceMessage() && bean.getSvcType() != null && bean.getSvcType().trim().length() > 0) {
      %>enabled, "<%=StringEncoderDecoder.encode(bean.getSvcType())%>"<%
    } else {
      %>disabled<%
    }
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=trackIntegrity>track integrity</label></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=trackIntegrity name=trackIntegrity value=true <%=bean.isTrackIntegrity() ? "checked" : ""%>><%
  } else {
    %><%=bean.isTrackIntegrity() ? "enabled" : "disabled"%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=keepHistory>keep messages history</label></th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=check type=checkbox id=keepHistory name=keepHistory value=true <%=bean.isKeepHistory() ? "checked" : ""%>><%
  } else {
    %><%=bean.isKeepHistory() ? "enabled" : "disabled"%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>system data source timeout</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=dsOwnTimeout value="<%=StringEncoderDecoder.encode(bean.getDsTimeout())%>">secs<%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getDsTimeout())%> secs<%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>messages cache size </th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=messagesCacheSize value="<%=StringEncoderDecoder.encode(bean.getMessagesCacheSize())%>"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getMessagesCacheSize())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>messages cache sleep</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=messagesCacheSleep value="<%=StringEncoderDecoder.encode(bean.getMessagesCacheSleep())%>">secs<%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getMessagesCacheSleep())%> secs<%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>uncommited in generation</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=uncommitedInGeneration value="<%=StringEncoderDecoder.encode(bean.getUncommitedInGeneration())%>"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getUncommitedInGeneration())%><%
  }%>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>uncommited in process</th>
  <td><%if (bean.isSmeRunning()) {
    %><input class=txt name=uncommitedInProcess value="<%=StringEncoderDecoder.encode(bean.getUncommitedInProcess())%>"><%
  } else {
    %><%=StringEncoderDecoder.encode(bean.getUncommitedInProcess())%><%
  }%>
  </td>
</tr>
</table>
</div><%
page_menu_begin(out);
if (bean.isSmeRunning()) {
  page_menu_button(out, "mbDone",  "Done",  "Done editing");
}
page_menu_button(out, "mbCancel", "Cancel", "Cancel changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>