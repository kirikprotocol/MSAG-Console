<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, ru.novosoft.smsc.util.Functions,
                 java.text.SimpleDateFormat"%>
<%@ page import="mobi.eyeline.smsquiz.beans.QuizView"%>
<%@ page import="mobi.eyeline.smsquiz.quizes.AnswerCategory"%>
<%@ page import="mobi.eyeline.smsquiz.quizes.view.QuizFullData"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.smsquiz.beans.QuizView" />
<jsp:setProperty name="bean" property="*"/>
<%
  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
  TITLE = getLocString("smsquiz.title");
  bean.getTableHelper().processRequest(request);
  int beanResult = bean.process(request);
  if(beanResult == QuizView.RESULT_DONE) {
    response.sendRedirect("quizes.jsp");
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<%

   QuizFullData quizData = bean.getQuizData();
%>
<div class=content>
<% int rowN = 0;%>
  <table class=properties_list cellspacing=0>
    <col width="1%">
    <col width="99%">
    <tr class=row<%=rowN++&1%>>
      <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.info")%></div></th>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th>
      <%=getLocString("smsquiz.label.quiz.status")%>
      </th>
      <td>
      <%=StringEncoderDecoder.encode(bean.getStatus())%>
      </td>
    </tr>
<%
  String reasonId = bean.getReasonId();
  if((reasonId!=null)&&(!reasonId.equals(""))) {
%>

    <tr class=row<%=rowN++&1%>>
      <th>
      <%=getLocString("smsquiz.label.quiz.reasonId")%>
      </th>
      <td><%=StringEncoderDecoder.encode(reasonId)%></td>
    </tr>

<%
  }
  String reason = bean.getReason();
  if((reason!=null)&&(!reason.equals(""))) {
        %>
    <tr class=row<%=rowN++&1%>>
      <th>
      <%=getLocString("smsquiz.label.quiz.reason")%>
      </th>
      <td><%=StringEncoderDecoder.encode(reason)%></td>
    </tr>
<%
  }
%></table>

<input type=hidden name=initialized value=true>

<table class=properties_list cellspacing=0>
<col width="1%">
<col width="99%">
<tr class=row<%=rowN++&1%>>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.general")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.quiz")%></th>
  <td><input class=txt readonly="true" name=quiz value="<%=StringEncoderDecoder.encode(bean.getQuiz())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.startDate")%></th>
  <td><%=StringEncoderDecoder.encode(quizData.getDateBegin())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.endDate")%></th>
  <td><%=StringEncoderDecoder.encode(quizData.getDateEnd())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.question")%></th>
  <td><%=StringEncoderDecoder.encode(quizData.getQuestion())%></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle>Distribution</div></th>
</tr>
  <tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.abfile")%></th>
  <td><%=StringEncoderDecoder.encode(quizData.getAbFile())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.sa")%></th>
  <td><%=StringEncoderDecoder.encode(quizData.getSourceAddress())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.startTime")%></th>
  <td><%=StringEncoderDecoder.encode(quizData.getTimeBegin())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.endTime")%></th>
  <td><%=StringEncoderDecoder.encode(quizData.getTimeEnd())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.activeDays")%></th>
  <td><%
    if(quizData.getActiveDays()!=null) {
      Iterator iter = quizData.getActiveDays().iterator();

      while(iter.hasNext()) {
        %><%=StringEncoderDecoder.encode((String)iter.next())%><%
        if(iter.hasNext()) {
        %><%=","%><%
        }
      }
    }
  %></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.txmode")%></th>
  <td><%=StringEncoderDecoder.encode(quizData.getTxmode())%></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.replies")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.da")%></th>
  <%if((bean.isFinished())||(bean.isActive())) { %>
    <td><%=StringEncoderDecoder.encode(quizData.getDestAddress())%></td>
  <%}else { %>
    <td><input validation="nonEmpty" class=txt name=destAddress value="<%=StringEncoderDecoder.encode(bean.getDestAddress())%>"></td>
  <%} %>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.maxrepeat")%></th>
  <%if(bean.isFinished()) { %>
    <td><%=StringEncoderDecoder.encode(quizData.getMaxRepeat())%></td>
  <%} else { %>
  <td><input validation="nonEmpty" class=txt name=maxRepeat value="<%=StringEncoderDecoder.encode(bean.getMaxRepeat())%>"></td>
  <%} %>
</tr><tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.defcat")%></th>
  <%if(bean.isFinished()) { %>
    <td><%=StringEncoderDecoder.encode(quizData.getDefaultCategory())%></td>
  <%} else { %>
  <td><input class=txt name=defaultCategory value="<%=StringEncoderDecoder.encode(bean.getDefaultCategory())%>"></td>
  <%} %>
</tr>
    <%if((bean.isFinished())||(bean.isActive())) {%>
      <tr>
        <th colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.category")%></div></th>
      </tr>
      <tr>
        <td align="center" colspan="2">
        <table class=list cellspacing=0>
        <tr>
          <th><%=getLocString("smsquiz.label.category.name")%></th>
          <th><%=getLocString("smsquiz.label.category.pattern")%></th>
          <th><%=getLocString("smsquiz.label.category.answer")%></th>
        </tr>
      <%
        Iterator categories = quizData.getCategoriesIter();
        while(categories.hasNext()) {
          AnswerCategory cat = (AnswerCategory) categories.next();
      %>
        <tr class=row<%=rowN++&1%>>
          <td><%=StringEncoderDecoder.encode(cat.getName())%></td>
          <td><%=StringEncoderDecoder.encode(cat.getPattern())%></td>
          <td><%=StringEncoderDecoder.encode(cat.getAnswer())%></td>
        </tr>
      <%  }

      %>
      </table>

      </td>
      </tr>
    <%} else { %>
      <tr>
        <td colspan="2"><%
          final DynamicTableHelper tableHelper = bean.getTableHelper();%>
          <%@ include file="/WEB-INF/inc/dynamic_table.jsp"%>
        </td>
      </tr>
    <%}%>
</table>

</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbDone",   "common.buttons.done",  "smsquiz.done_editing");
  page_menu_button(session, out, "mbCancel",   "common.buttons.cancel",  "smsquiz.cancel");
  page_menu_space(out);
  page_menu_end(out);

%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>