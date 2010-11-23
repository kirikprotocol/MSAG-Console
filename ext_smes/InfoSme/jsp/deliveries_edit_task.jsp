<%@ page import="ru.novosoft.smsc.infosme.backend.config.tasks.Task" %>
<% int rowN = 0;%>
<div class=content>

  <% 
    Collection regions = deliveries_bean.getRegionIds();
    if(!regions.isEmpty()) {%>
  <div class=page_subtitle><%=getLocString("infosme.label.abonentsNumber")%></div>
  <table class=properties_list border="0">
    <col width="10%">
    <%for(Iterator regionIter=regions.iterator(); regionIter.hasNext();) {
      Integer region = (Integer)regionIter.next();
      int actualSize = deliveries_bean.getActualSize(region.intValue());%>
    <tr class=row<%=rowN++&1%>><td><%=deliveries_bean.getRegionName(region.intValue())%></td><td><%=actualSize%></td></tr>
    <%}%>
  </table>
  <br/>
  <%}%>

  <div class=page_subtitle><%= getLocString("infosme.subtitle.stage3")%></div><br/>
  <input type=hidden id="oldActiveTaskRegionId" name="oldActiveTaskRegionId" value="<%=deliveries_bean.getActiveTaskRegionId()%>"/>

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

    <% if (deliveries_bean.isSplitDeliveriesFile() && !deliveries_bean.isUseSameSettingsForAllRegions()) { %>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.region")%></th>
      <td>
        <select id="activeTaskRegionId" name="activeTaskRegionId" onchange="return opFormSubmit();">
          <%
            for (Iterator iter = deliveries_bean.getRegionIds().iterator(); iter.hasNext();) {
              int regionId = ((Integer)iter.next()).intValue();
              boolean selected = regionId == deliveries_bean.getActiveTaskRegionId();
          %>
          <option value="<%=regionId%>" <%=selected ? "selected" : ""%>><%=bean.getRegionName(regionId)%></option>
          <%
            }
          %>
        </select>
      </td>
    </tr>
    <% } %>
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
        <input class=txt id=validityPeriod name=validityPeriod value="<%=StringEncoderDecoder.encode(deliveries_bean.getValidityPeriod())%>" maxlength=20 style="z-index:22;">
        &nbsp;<%= getLocString("infosme.label.vperiod_date_or")%>&nbsp;
        <input class=calendarField id=validityDate name=validityDate value="<%=StringEncoderDecoder.encode(deliveries_bean.getValidityDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(validityDate, false, true);">...</button>
      </td>
    </tr>
<% }%>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.start_date")%></th>
      <td nowrap>
        <input class=calendarField id=startDate name=startDate value="<%=StringEncoderDecoder.encode(deliveries_bean.getStartDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(startDate, false, true);">...</button>
        <% if (deliveries_bean.isUseSameSettingsForAllRegions()) { %>
        &nbsp;
        <input type="checkbox" name="startDateInLocalTime" id="startDateInLocalTime" <%=deliveries_bean.isStartDateInLocalTime() ? "checked" : ""%>/>
        <%= getLocString("infosme.label.start_date_in_local_time")%>
        <% } %>
      </td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.end_date")%></th>
      <td nowrap>
        <input class=calendarField id=endDate name=endDate value="<%=StringEncoderDecoder.encode(deliveries_bean.getEndDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(endDate, false, true);">...</button>
        <% if (deliveries_bean.isUseSameSettingsForAllRegions()) { %>
        &nbsp;
        <input type="checkbox" name="endDateInLocalTime" id="endDateInLocalTime" <%=deliveries_bean.isEndDateInLocalTime() ? "checked" : ""%>/>
        <%= getLocString("infosme.label.end_date_in_local_time")%>
        <% } %>
      </td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><label for=retryOnFail><%= getLocString("infosme.label.retry_on_fail")%></label></th>
      <td nowrap>
        <input class=check type=checkbox id=retryOnFail name=retryOnFail value=true <%=deliveries_bean.isRetryOnFail() ? "checked" : ""%>>
        <select id="retryPolicy" name="retryPolicy">
          <%for (Iterator iter = deliveries_bean.getRetryPolicies().iterator(); iter.hasNext();) {
            String policy = (String)iter.next();
          %>
          <option value="<%=policy%>" <%=deliveries_bean.getRetryPolicy() != null && deliveries_bean.getRetryPolicy().equals(policy) ? "SELECTED" : ""%>><%=StringEncoderDecoder.encode(policy)%></option>
          <%}%>
        </select>
      </td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><label for="transactionMode"><%= getLocString("infosme.label.transaction_mode")%></label></th>
      <td><input class=check type=checkbox id=transactionMode name=transactionMode <%=deliveries_bean.isTransactionMode() ? "checked" : ""%>></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><label for="useDataSm"><%= getLocString("infosme.label.use_data_sm")%></label></th>
      <td><input class=check type=checkbox id=useDataSm name=useDataSm <%=deliveries_bean.isUseDataSm() ? "checked" : ""%>></td>
    </tr>
<% if ( deliveries_bean.isUssdPushAllowed() ) { %>
    <tr class=row<%=rowN++&1%>>
      <th><label for="deliveryMode"><%= getLocString("infosme.label.delivery_mode")%></label></th>
      <td>
        <select id="deliveryMode" name="deliveryMode">
          <option value="<%=Task.DELIVERY_MODE_SMS%>" <%=bean.getDeliveryMode() == Task.DELIVERY_MODE_SMS ? "selected" : ""%>> <%= getLocString("infosme.label.delivery_mode_sms")%></option>
          <option value="<%=Task.DELIVERY_MODE_USSD_PUSH%>" <%=bean.getDeliveryMode() == Task.DELIVERY_MODE_USSD_PUSH ? "selected" : ""%>><%= getLocString("infosme.label.delivery_mode_ussd_push")%></option>
          <option value="<%=Task.DELIVERY_MODE_USSD_PUSH_VLR%>" <%=bean.getDeliveryMode() == Task.DELIVERY_MODE_USSD_PUSH_VLR ? "selected" : ""%>><%= getLocString("infosme.label.delivery_mode_ussd_push_vlr")%></option>
        </select> 
      </td>
    </tr>
<% } %>
<% if (!deliveries_bean.isFileContainsText()) { %>
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
  <% if (deliveries_bean.isSupportExtProfile()) {%>
    <script type="text/javascript">
      function checkSecretText() {
        document.getElementById("secretText").disabled = !document.getElementById("secret").checked;
        document.getElementById("secretFlash").disabled = !document.getElementById("secret").checked;
      }
    </script>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.secret")%></th>
      <td><input class=check type=checkbox id=secret name=secret value=true <%=deliveries_bean.isSecret() ? "checked" : ""%> onchange="checkSecretText()"></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.secret_message_text")%></th>
      <td><textarea name=secretText id="secretText"><%=StringEncoderDecoder.encode(deliveries_bean.getSecretText())%></textarea></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.secret_flash")%></th>
      <td><input class=check type=checkbox id=secretFlash name=secretFlash value=true <%=deliveries_bean.isSecretFlash() ? "checked" : ""%> onchange="checkSecretText()"></td>
    </tr>
    <script type="text/javascript">
      checkSecretText();
    </script>
  <% } %>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.test_sms")%></th>
      <td><input class=txt id=testSmsAddress name=testSmsAddress value="<%=deliveries_bean.getTestSmsAddress()%>" ></td>
    </tr>
<% } else { %>
    <tr class=row<%=rowN++&1%>>
     <th><%= getLocString("infosme.label.message_text")%></th>
     <td><%= getLocString("infosme.msg.text_included_in_file")%></td>
   </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("infosme.label.flash")%></th>
      <td><input class=check type=checkbox id=flash name=flash value=true <%=deliveries_bean.isFlash() ? "checked" : ""%>></td>
    </tr>
<% } %>
  </table>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel",    "infosme.hint.cancel_task_add", "clickCancel()");
  page_menu_button(session, out, "mbTest",   "common.buttons.test", "");
  page_menu_button(session, out, "mbNext",   "infosme.button.next_page", "infosme.hint.next_page");
  page_menu_space(out);
  page_menu_end(out);
%>
