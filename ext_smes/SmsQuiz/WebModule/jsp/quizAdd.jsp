<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder"%>
<%@ page import="mobi.eyeline.smsquiz.beans.QuizAdd"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper"%>
<%@ page import="ru.novosoft.util.jsp.MultipartServletRequest"%>
<%@ page import="ru.novosoft.util.jsp.MultipartDataSource"%>
<%@ page import="java.io.*"%>
<%@ page import="ru.novosoft.smsc.util.Functions"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.smsquiz.beans.QuizAdd" />
<%

  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
  if (request.getAttribute("multipart.request") != null) {
    request = (MultipartServletRequest)request.getAttribute("multipart.request");
  }
%>
<jsp:setProperty name="bean" property="*"/>
<%

  bean.getTableHelper().processRequest(request);

  TITLE=getLocString("smsquiz.title");
  MENU0_SELECTION = "MENU0_SERVICES";
  FORM_METHOD = "POST";
  FORM_ENCTYPE = "multipart/form-data";
  FORM_URI = CPATH +"/upload";

  int beanResult = bean.process(request);
  switch(beanResult)
  {
    case QuizAdd.RESULT_DONE:
      response.sendRedirect(CPATH+"/esme_SmsQuiz/quizes.jsp");
      return;
  }
  int rowN = 0;
  if (request.getAttribute("multipart.request") != null) {
    System.out.println("multipart exist");
    //response.sendRedirect(request.getContextPath() + "/smsc/esme_SmsQuiz/quizAdd.jsp");
  }

%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="/WEB-INF/inc/time.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>

<table class=properties_list cellpadding="0">
<col width="1%">
<col width="99%">
<tr>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.general")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.quiz")%></th>
  <td><input class=txt name=quiz validation="nonEmpty" value="<%=StringEncoderDecoder.encode(bean.getQuiz())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.startDate")%></th>
  <td><input validation="nonEmpty" class=calendarField id=fromDate name=dateBegin value="<%=StringEncoderDecoder.encode(bean.getDateBegin())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.endDate")%></th>
  <td><input validation="nonEmpty" class=calendarField id=tillDate name=dateEnd value="<%=StringEncoderDecoder.encode(bean.getDateEnd())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.question")%></th>
  <td><input validation="nonEmpty" class=txt name=question value="<%=StringEncoderDecoder.encode(bean.getQuestion())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.distribution")%></div></th>
</tr>
  <tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.abfile")%></th>
  <td><input type="hidden" name="jsp" value="/esme_SmsQuiz/quizAdd.jsp">
  <input class=txt type=file name=file id=file></td>
 <!--<td><input validation="nonEmpty" class=txt name=abFile value="<%=StringEncoderDecoder.encode(bean.getAbFile())%>"></td> -->
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.sa")%></th>
  <td><input validation="nonEmpty" class=txt name=sourceAddress value="<%=StringEncoderDecoder.encode(bean.getSourceAddress())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.startTime")%></th>
  <td>
    <input validation="nonEmpty" class=timeField id=activePeriodStart name=timeBegin value="<%=StringEncoderDecoder.encode(bean.getTimeBegin())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodStart, false, true);">...</button>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.endTime")%></th>
    <td>
    <input validation="nonEmpty" class=timeField id=activePeriodEnd name=timeEnd value="<%=StringEncoderDecoder.encode(bean.getTimeEnd())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(activePeriodEnd, false, true);">...</button>
  </td>

  </tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.activeDays")%></th>
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
  <th><%=getLocString("smsquiz.label.txmode")%></th>
  <td><input validation="nonEmpty" class=txt name=txMode value="<%=StringEncoderDecoder.encode(bean.getTxmode())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.replies")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.da")%></th>
  <td><input validation="nonEmpty" class=txt name=destAddress value="<%=StringEncoderDecoder.encode(bean.getDestAddress())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.maxrepeat")%></th>
  <td><input validation="nonEmpty" class=txt name=maxRepeat value="<%=StringEncoderDecoder.encode(bean.getMaxRepeat())%>"></td>
</tr><tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.defcat")%></th>
  <td><input class=txt name=defaultCategory value="<%=StringEncoderDecoder.encode(bean.getDefaultCategory())%>"></td>
</tr>
 <tr>
  <td colspan="2">
    <%{ final DynamicTableHelper tableHelper = bean.getTableHelper();%>
    <%@ include file="/WEB-INF/inc/dynamic_table.jsp"%>
    <%}%>
  </td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",  "smsquiz.done_editing");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "smsquiz.cancel_changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>