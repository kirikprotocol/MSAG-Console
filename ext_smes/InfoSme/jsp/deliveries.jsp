<%@ include file="/WEB-INF/inc/code_header.jsp"%><%@
    page import="ru.novosoft.smsc.infosme.beans.Deliveries,
                 ru.novosoft.smsc.jsp.SMSCJspException, ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.infosme.beans.InfoSmeBean, ru.novosoft.smsc.jsp.PageBean,
                 ru.novosoft.util.jsp.MultipartServletRequest"%><jsp:useBean id="deliveries_bean" scope="session" class="ru.novosoft.smsc.infosme.beans.Deliveries" /><jsp:setProperty name="deliveries_bean" property="*"/><%

    TITLE=getLocString("infosme.title");
    ru.novosoft.smsc.infosme.beans.Deliveries bean = deliveries_bean;

    MultipartServletRequest multi = (MultipartServletRequest)request.getAttribute("multipart.request");
    if (multi != null) request = multi;
    if (bean.getStage() == 2 && request.getParameter("jsp") != null)
	    FORM_URI = CPATH + request.getParameter("jsp");

    switch(bean.process(request))
    {
    case PageBean.RESULT_OK:
    case PageBean.RESULT_ERROR:
        break;
    case InfoSmeBean.RESULT_STAT:
        response.sendRedirect("stat.jsp");
        return;
    case InfoSmeBean.RESULT_DLSTAT:
        response.sendRedirect("stat.jsp?csv=true");
        return;
    case InfoSmeBean.RESULT_DONE:
        if (request.isUserInRole(InfoSmeBean.INFOSME_ADMIN_ROLE)) {
            response.sendRedirect("index.jsp");
        } else {
            response.sendRedirect("deliveries.jsp");
        }
        return;
    default:
        errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction,
                                               SMSCJspException.ERROR_CLASS_ERROR));
    }

System.out.println("Stage="+bean.getStage()+", multi="+(multi == null ? "null":multi.toString()));
if (bean.getStage() == 1 && multi != null) {
    int result = bean.receiveFile((MultipartServletRequest)request.getAttribute("multipart.request"));
    System.out.println("RCV result="+result);
}
if (bean.getStage() == 1) {
	FORM_URI = CPATH+"/upload";
	FORM_METHOD = "POST";
	FORM_ENCTYPE = "multipart/form-data";
}
else {
	FORM_URI = CPATH+"/esme_InfoSme/deliveries.jsp";
	FORM_ENCTYPE = "application/x-www-form-urlencoded";
}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="/WEB-INF/inc/time.jsp"%>
<%if (bean.getStage() == 3 && isBrowserMSIE(request) ) {%>
<OBJECT id="tdcProgress" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
	<PARAM NAME="DataURL" VALUE="/smsc/smsc/esme_InfoSme/taskProgress.jsp">
	<PARAM NAME="UseHeader" VALUE="True">
	<PARAM NAME="TextQualifier" VALUE='"'>
</OBJECT>
<%}%>
<div class=content>
<%switch (bean.getStage())
{
    case 0:
        {%>
<div class=page_subtitle><%= getLocString("infosme.subtitle.welcome")%></div><br/>
<div class=secInfo><%= getLocString("infosme.label.phraze1")%></div>
<div class=secInfo><%= getLocString("infosme.label.phraze2")%></div>
<div class=secInfo><%= getLocString("infosme.label.phraze3")%></div>
        <%}
        break;

    /* ######################### start page (abonents list file) ########################### */
    case 1:
        {%>
<input type="hidden" name="jsp" value="/esme_InfoSme/deliveries.jsp">
<div class=page_subtitle><%= getLocString("infosme.subtitle.stage1")%></div><br/>
<table class=properties_list cellspacing=0 cellpadding=0>
<col width="15%" align=right>
<col width="85%">
<tr class=row0>
    <th><%= getLocString("infosme.label.file_location")%></th>
    <td><input class=txt type=file name="<%=Deliveries.ABONENTS_FILE_PARAM%>" id="<%=Deliveries.ABONENTS_FILE_PARAM%>" onChange="checkCreateButton();"></td>
</tr>
</table>
        <%}
        break;

    /* ################################## task parameters ################################## */
    case 2:
        { int rowN = 0;%>
<div class=page_subtitle><%= getLocString("infosme.subtitle.stage2")%></div><br/>
<table class=properties_list>
<col width="10%">
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.task_id")%></th>
  <td><input class=txt name=id value="<%=StringEncoderDecoder.encode(bean.getId())%>" maxlength="5" validation="id" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.task_name")%></th>
  <td><input class=txt name=name value="<%=StringEncoderDecoder.encode(bean.getName())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=transactionMode><%= getLocString("infosme.label.transaction_mode")%></label></th>
  <td><input class=check type=checkbox id=transactionMode name=transactionMode value=true <%=bean.isTransactionMode() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.active_period")%></th>
  <td>
    <input class=timeField id=activePeriodStart name=activePeriodStart value="<%=StringEncoderDecoder.encode(bean.getActivePeriodStart())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodStart, false, true);">...</button>
    &nbsp;<%= getLocString("infosme.label.active_period_to")%>&nbsp;
    <input class=timeField id=activePeriodEnd name=activePeriodEnd value="<%=StringEncoderDecoder.encode(bean.getActivePeriodEnd())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodEnd, false, true);">...</button>
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
    </table>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.vperiod_date")%></th>
  <td>
    <input class=timeField id=validityPeriod name=validityPeriod value="<%=StringEncoderDecoder.encode(bean.getValidityPeriod())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(validityPeriod, false, true);">...</button>
    &nbsp;<%= getLocString("infosme.label.vperiod_date_or")%>&nbsp;
    <input class=calendarField id=validityDate name=validityDate value="<%=StringEncoderDecoder.encode(bean.getValidityDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(validityDate, false, true);">...</button>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.end_date")%></th>
  <td nowrap>
    <input class=calendarField id=endDate name=endDate value="<%=StringEncoderDecoder.encode(bean.getEndDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(endDate, false, true);">...</button>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.message_text")%></th>
  <td><textarea name=text><%=StringEncoderDecoder.encode(bean.getText())%></textarea></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("infosme.label.transliterate_text")%></th>
  <td><input class=check type=checkbox id=transliterate name=transliterate value=true <%=bean.isTransliterate() ? "checked" : ""%>></td>
</tr>
</table>
        <%}
        break;

    /* ########################## wait task create completion ... ######################### */
    case 3:
        {%>
<div class=page_subtitle><%= getLocString("infosme.subtitle.stage3")%></div><br/>
<div class=secInfo><%= getLocString("infosme.label.status")%>&nbsp;<span id=tdcStatus datasrc=#tdcProgress DATAFORMATAS=html datafld="status" style='color:blue;'><%= bean.getStatusStr()%></span></div>
<div class=secInfo><%= getLocString("infosme.label.messages_generated")%>&nbsp;<span datasrc=#tdcProgress DATAFORMATAS=html datafld="messages"><%= bean.getMessages()%></span></div>
<div class=secInfo><%= getLocString("infosme.label.total_progress")%>&nbsp;<span datasrc=#tdcProgress DATAFORMATAS=html datafld="progress"><%= bean.getProgress()%></span>%</div>
<script type="text/javascript">
function refreshProgressStatus()
{
	document.getElementById('tdcProgress').DataURL = document.getElementById('tdcProgress').DataURL;
	document.getElementById('tdcProgress').reset();
    if (document.getElementById('tdcStatus').innerText != null &&
        document.getElementById('tdcStatus').innerText == 'Finished') document.getElementById('mbNext').disabled = false;
    else if (document.getElementById('tdcStatus').innerText != null &&
        document.getElementById('tdcStatus').innerText.match('Error') != null) document.getElementById('mbCancel').innerText = 'Abort';
	else window.setTimeout(refreshProgressStatus, 500);
}
refreshProgressStatus();
</script>
        <%}
        break;

    /* ########################### add task to config & service ########################### */
    case 4:
        {%>
<div class=page_subtitle><%= getLocString("infosme.subtitle.stage4")%></div><br/>
<div class=secInfo><%= ((bean.getStatus() == bean.STATUS_OK) ? 
                         getLocString("infosme.msg.task_add_success"):
                         getLocString("infosme.msg.task_add_failure"))%>'<%= bean.getId()%>'</div>
<div class=secInfo><%= getLocString("infosme.msg.to_main")%></div>
        <%}
        break;

	default:
	{
	}
}%>
</div> <!-- content -->
<%switch (bean.getStage())
{
    case 0:
        page_menu_begin(out);
        page_menu_button(session, out, "mbNext",   "infosme.button.create_delivery",     "infosme.hint.create_delivery");
        page_menu_button(session, out, "mbStat",   "infosme.button.view_statistics",     "infosme.hint.view_statistics");
        page_menu_button(session, out, "mbDlstat", "infosme.button.download_statistics", "infosme.hint.download_statistics");
        page_menu_space(out);
        page_menu_end(out);
        break;
    case 1:
        page_menu_begin(out);
        //page_menu_button(session, out, "mbCancel", "common.buttons.cancel",    "infosme.hint.cancel_task_add", "clickCancel()");
        page_menu_button(session, out, "mbNext",   "infosme.button.next_page", "infosme.hint.next_page", false);
        page_menu_space(out);
        page_menu_end(out);
        %><script type="text/javascript">
        function checkCreateButton() {
          document.getElementById('mbNext').disabled = (document.getElementById('<%=Deliveries.ABONENTS_FILE_PARAM%>').value.length <= 0);
        }</script><%
        break;
    case 2:
        page_menu_begin(out);
        page_menu_button(session, out, "mbCancel", "common.buttons.cancel",    "infosme.hint.cancel_task_add", "clickCancel()");
        page_menu_button(session, out, "mbNext",   "infosme.button.next_page", "infosme.hint.next_page");
        page_menu_space(out);
        page_menu_end(out);
        break;
    case 3:
        page_menu_begin(out);
        page_menu_button(session, out, "mbCancel", "common.buttons.cancel",    "infosme.hint.cancel_task_add", "clickCancel()");
        page_menu_button(session, out, "mbNext",   "infosme.button.next_page", "infosme.hint.finish_task_add", false);
        page_menu_space(out);
        page_menu_end(out);
        break;
    case 4:
        page_menu_begin(out);
        page_menu_button(session, out, "mbNext", "common.buttons.done", "infosme.hint.main_page");
        page_menu_space(out);
        page_menu_end(out);
        break;
    default:
        break;
}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>