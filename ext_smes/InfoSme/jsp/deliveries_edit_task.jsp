<%@ page import="ru.novosoft.smsc.admin.route.Subject"%>

<% int rowN = 0;%>
<div class=content>
  <div class=page_subtitle><%= getLocString("infosme.subtitle.stage3")%></div><br/>
  <input type=hidden id="oldActiveTaskSubject" name="oldActiveTaskSubject" value="<%=deliveries_bean.getActiveTaskSubject()%>"/>

  <table class=properties_list>
    <col width="10%">
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.task_name")%></th>
      <td><input class=txt name=name value="<%=StringEncoderDecoder.encode(deliveries_bean.getName())%>"></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.source_address")%></th>
      <td><input class=txt name=sourceAddress value="<%=StringEncoderDecoder.encode(deliveries_bean.getSourceAddress())%>"></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.subject")%></th>
      <td>
        <select id="activeTaskSubject" name="activeTaskSubject" onchange="return opFormSubmit();">
          <%
            for (Iterator iter = deliveries_bean.getSubjects().iterator(); iter.hasNext();) {
              String subject = (String)iter.next();
              boolean selected = subject.equalsIgnoreCase(deliveries_bean.getActiveTaskSubject());
          %>
          <option value="<%=subject%>" <%=selected ? "selected" : ""%>><%=subject%></option>
          <%
            }
          %>
        </select>
      </td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.abonentsNumber")%></th>
      <td><%=StringEncoderDecoder.encode(deliveries_bean.getRecordsNumber())%></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.active_period")%></th>
      <td>
        <input class=timeField id=activePeriodStart name=activePeriodStart value="<%=StringEncoderDecoder.encode(deliveries_bean.getActivePeriodStart())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodStart, false, true);">...</button>
        &nbsp;<%= getLocString("infosme.label.active_period_to")%>&nbsp;
        <input class=timeField id=activePeriodEnd name=activePeriodEnd value="<%=StringEncoderDecoder.encode(deliveries_bean.getActivePeriodEnd())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodEnd, false, true);">...</button>
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
            <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Mon value=Mon <%=deliveries_bean.isWeekDayActive("Mon") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Mon>Monday</label></td>
            <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Thu value=Thu <%=deliveries_bean.isWeekDayActive("Thu") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Thu>Thursday</label></td>
            <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Sat value=Sat <%=deliveries_bean.isWeekDayActive("Sat") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Sat>Saturday</label></td>
          <tr>
          <tr>
          <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Tue value=Tue <%=deliveries_bean.isWeekDayActive("Tue") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Tue>Tuesday</label></td>
          <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Fri value=Fri <%=deliveries_bean.isWeekDayActive("Fri") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Fri>Friday</label></td>
          <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Sun value=Sun <%=deliveries_bean.isWeekDayActive("Sun") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Sun>Sunday</label></td>
          <tr>
          <tr>
          <td style="border:none;"><input class=check type=checkbox name=activeWeekDays id=activeWeekDays_Wed value=Wed <%=deliveries_bean.isWeekDayActive("Wed") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Wed>Wednesday</label></td>
          <tr>
        </table>
      </td>
    </tr>
<% if (deliveries_bean.isUserAdmin(request)) {%>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.vperiod_date")%></th>
      <td>
        <input class=timeField id=validityPeriod name=validityPeriod value="<%=StringEncoderDecoder.encode(deliveries_bean.getValidityPeriod())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(validityPeriod, false, true);">...</button>
        &nbsp;<%= getLocString("infosme.label.vperiod_date_or")%>&nbsp;
        <input class=calendarField id=validityDate name=validityDate value="<%=StringEncoderDecoder.encode(deliveries_bean.getValidityDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(validityDate, false, true);">...</button>
      </td>
    </tr>
<% }%>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.start_date")%></th>
      <td nowrap>
        <input class=calendarField id=startDate name=startDate value="<%=StringEncoderDecoder.encode(deliveries_bean.getStartDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(startDate, false, true);">...</button>
      </td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.end_date")%></th>
      <td nowrap>
        <input class=calendarField id=endDate name=endDate value="<%=StringEncoderDecoder.encode(deliveries_bean.getEndDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(endDate, false, true);">...</button>
      </td>
    </tr>
<% if (deliveries_bean.isUserAdmin(request)) { %>
    <tr class=row<%=rowN++&1%>>
      <th><label for=retryOnFail><%= getLocString("infosme.label.retry_on_fail")%></label></th>
      <td nowrap>
        <input class=check type=checkbox id=retryOnFail name=retryOnFail value=true <%=deliveries_bean.isRetryOnFail() ? "checked" : ""%> onClick="document.getElementById('retryPolicy').disabled = !this.checked;">
        <select id="retryPolicy" name="retryPolicy">
          <%for (Iterator iter = deliveries_bean.getRetryPolicies().iterator(); iter.hasNext();) {
            String policy = (String)iter.next();
          %>
          <option value="<%=policy%>" <%=deliveries_bean.getRetryPolicy() != null && deliveries_bean.getRetryPolicy().equals(policy) ? "SELECTED" : ""%>><%=StringEncoderDecoder.encode(policy)%></option>
          <%}%>
        </select>
        <script>document.getElementById('retryPolicy').disabled = !document.getElementById('retryOnFail').checked;</script>
      </td>
    </tr>
<% } %>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.message_text")%></th>
      <td><textarea name=text><%=StringEncoderDecoder.encode(deliveries_bean.getText())%></textarea></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.transliterate_text")%></th>
      <td><input class=check type=checkbox id=transliterate name=transliterate value=true <%=deliveries_bean.isTransliterate() ? "checked" : ""%>></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.flash")%></th>
      <td><input class=check type=checkbox id=flash name=flash value=true <%=deliveries_bean.isFlash() ? "checked" : ""%>></td>
    </tr>
  </table>
</div>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel",    "infosme.hint.cancel_task_add", "clickCancel()");
  page_menu_button(session, out, "mbNext",   "infosme.button.next_page", "infosme.hint.next_page");
  page_menu_space(out);
  page_menu_end(out);
%>