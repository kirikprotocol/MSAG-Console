<%@ page import="mobi.eyeline.smsquiz.DistributionHelper"%>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.sa")%></th>
  <td><input validation="nonEmpty" class=txt name="<%=distributionHelper.getUid()+DistributionHelper.SA_FIELD%>"
             value="<%=StringEncoderDecoder.encode(distributionHelper.getSourceAddress())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.startTime")%></th>
  <td>
    <input validation="nonEmpty" class=timeField id="<%=distributionHelper.getUid()+DistributionHelper.TIMEBEGIN_FIELD%>"
           name="<%=distributionHelper.getUid()+DistributionHelper.TIMEBEGIN_FIELD%>" value="<%=StringEncoderDecoder.encode(distributionHelper.getTimeBegin())%>"
        maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(<%=distributionHelper.getUid()+DistributionHelper.TIMEBEGIN_FIELD%>, false, true);">...</button>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.endTime")%></th>
    <td>
    <input validation="nonEmpty" class=timeField id="<%=distributionHelper.getUid()+DistributionHelper.TIMEEND_FIELD%>"
           name="<%=distributionHelper.getUid()+DistributionHelper.TIMEEND_FIELD%>" value="<%=StringEncoderDecoder.encode(distributionHelper.getTimeEnd())%>"
               maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(<%=distributionHelper.getUid()+DistributionHelper.TIMEEND_FIELD%>, false, true);">...</button>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.endDate")%></th>
  <td><input class=calendarField id="<%=distributionHelper.getUid()+DistributionHelper.DATEEND_FIELD%>"
             name="<%=distributionHelper.getUid()+DistributionHelper.DATEEND_FIELD%>" value="<%=StringEncoderDecoder.encode(distributionHelper.getDistrDateEnd())%>"
             maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(<%=distributionHelper.getUid()+DistributionHelper.DATEEND_FIELD%>, false, true);">...</button></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.activeDays")%></th>
  <td>
  <table>
    <col width="1%"><col width="32%">
    <col width="1%"><col width="32%">
    <col width="1%"><col width="32%">
    <tr>
      <td style="border:none;"><input class=check type=checkbox name="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>"
                                      id="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Mon"
                                      value=Mon <%=distributionHelper.isWeekDayActive("Mon") ? "checked" : ""%>></td><td style="border:none;">
      <label for=<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>>Monday</label></td>
      <td style="border:none;"><input class=check type=checkbox name="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>"
                                      id="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Thu"
                                      value=Thu <%=distributionHelper.isWeekDayActive("Thu") ? "checked" : ""%>></td><td style="border:none;">
      <label for=<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Thu>Thursday</label></td>
      <td style="border:none;"><input class=check type=checkbox name="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>"
                                      id="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Sat"
                                      value=Sat <%=distributionHelper.isWeekDayActive("Sat") ? "checked" : ""%>></td><td style="border:none;">
      <label for=<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Sat>Saturday</label></td>
    <tr>
    <tr>
      <td style="border:none;"><input class=check type=checkbox name="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>"
                                      id="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Tue"
                                      value=Tue <%=distributionHelper.isWeekDayActive("Tue") ? "checked" : ""%>></td><td style="border:none;">
    <label for=<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Tue>Tuesday</label></td>
      <td style="border:none;"><input class=check type=checkbox name="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>"
                                      id="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Fri"
                                      value=Fri <%=distributionHelper.isWeekDayActive("Fri") ? "checked" : ""%>></td><td style="border:none;">
    <label for=<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Fri>Friday</label></td>
      <td style="border:none;"><input class=check type=checkbox name="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>"
                                      id="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Sun"
                                      value=Sun <%=distributionHelper.isWeekDayActive("Sun") ? "checked" : ""%>></td><td style="border:none;">
    <label for=<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Sun>Sunday</label></td>
    <tr>
    <tr>
      <td style="border:none;"><input class=check type=checkbox name="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>"
                                      id="<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Wed"
                                      value=Wed <%=distributionHelper.isWeekDayActive("Wed") ? "checked" : ""%>></td><td style="border:none;">
    <label for=<%=distributionHelper.getUid()+DistributionHelper.DAYS_FIELD%>_Wed>Wednesday</label></td>
    <tr>
    </table>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.txmode")%></th>
  <td><input type=checkbox class=check value="true" id="<%=distributionHelper.getUid()+DistributionHelper.TXMODE_FIELD%>"
             name=<%=distributionHelper.getUid()+DistributionHelper.TXMODE_FIELD%> <%=distributionHelper.isTxmode() ? "checked" : ""%>></td>
</tr>