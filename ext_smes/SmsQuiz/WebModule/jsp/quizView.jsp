<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, ru.novosoft.smsc.util.Functions,
                 java.text.SimpleDateFormat"%>
<%@ page import="mobi.eyeline.smsquiz.beans.QuizView"%>
<%@ page import="mobi.eyeline.smsquiz.quizes.AnswerCategory"%>
<%@ page import="mobi.eyeline.smsquiz.quizes.view.QuizFullData"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.smsquiz.beans.QuizView" />
<jsp:setProperty name="bean" property="*"/>
<%
   ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	 TITLE = getLocString("smsquiz.title");
   int beanResult = bean.process(request);
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<%
   if(beanResult== QuizView.RESULT_OK) {
   QuizFullData quizData = bean.getQuizData();
%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0>
<col width="1%">
<col width="99%">
<% int rowN = 0;%>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle>General</div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Quiz</th>
  <td><input class=txt name=quiz value="<%=StringEncoderDecoder.encode(bean.getQuiz())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Start date</th>
  <td><%=StringEncoderDecoder.encode(quizData.getDateBegin())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>End date</th>
  <td><%=StringEncoderDecoder.encode(quizData.getDateEnd())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Question</th>
  <td><%=StringEncoderDecoder.encode(quizData.getQuestion())%></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle>Distribution</div></th>
</tr>
  <tr class=row<%=rowN++&1%>>
  <th>Abonents file</th>
  <td><%=StringEncoderDecoder.encode(quizData.getAbFile())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Source address</th>
  <td><%=StringEncoderDecoder.encode(quizData.getSourceAddress())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Start time</th>
  <td><%=StringEncoderDecoder.encode(quizData.getTimeBegin())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>End time</th>
  <td><%=StringEncoderDecoder.encode(quizData.getTimeEnd())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Active days</th>
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
  <th>Tx mode</th>
  <td><%=StringEncoderDecoder.encode(quizData.getTxmode())%></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle>Replies</div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Dest address</th>
  <td><%=StringEncoderDecoder.encode(quizData.getDestAddress())%></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Max repeat</th>
  <td><%=StringEncoderDecoder.encode(quizData.getMaxRepeat())%></td>
</tr><tr class=row<%=rowN++&1%>>
  <th>Default category</th>
  <td><%=StringEncoderDecoder.encode(quizData.getDefaultCategory())%></td>
</tr>
<%
  Iterator categories = quizData.getCategories();
  while(categories.hasNext()) {
    AnswerCategory cat = (AnswerCategory) categories.next();
%>
    <tr>
      <th align="center" colspan="2"><div class=page_subtitle>Category</div></th>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th>Name</th>
      <td><%=StringEncoderDecoder.encode(cat.getName())%></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th>Pattern</th>
      <td><%=StringEncoderDecoder.encode(cat.getPattern())%></td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th>Answer</th>
      <td><%=StringEncoderDecoder.encode(cat.getAnswer())%></td>
    </tr>
<%  }

%>

</table>

</div>
<%
  }
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>