<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, ru.novosoft.smsc.util.Functions,
                 java.text.SimpleDateFormat"%>
<%@ page import="mobi.eyeline.smsquiz.beans.QuizView"%>
<%@ page import="mobi.eyeline.smsquiz.quizes.AnswerCategory"%>
<%@ page import="mobi.eyeline.smsquiz.quizes.view.QuizFullData"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.smsquiz.beans.QuizEdit" />
<jsp:setProperty name="bean" property="*"/>
<%
  bean.getTableHelper().processRequest(request);
  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
  TITLE = getLocString("smsquiz.title");
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
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.general")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.quiz")%></th>
  <td><input class=txt readonly="true" name=quiz value="<%=StringEncoderDecoder.encode(bean.getQuiz())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.replies")%></div></th>
</tr>
<% if(!bean.isActive()) { %>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.da")%></th>
  <td><input validation="nonEmpty" class=txt name=destAddress value="<%=StringEncoderDecoder.encode(bean.getDestAddress())%>"></td>
</tr>
<%}%>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.maxrepeat")%></th>
  <td><input validation="nonEmpty" class=txt name=maxRepeat value="<%=StringEncoderDecoder.encode(bean.getMaxRepeat())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.defcat")%></th>
  <td><input class=txt name=defaultCategory value="<%=StringEncoderDecoder.encode(bean.getDefaultCategory())%>"></td>
</tr>
<% if(!bean.isActive()) { %>
  <tr>
    <td colspan="2">
    <%{ final DynamicTableHelper tableHelper = bean.getTableHelper();%>
      <%@ include file="/WEB-INF/inc/dynamic_table.jsp"%>
    <%}%>
    </td>
  </tr>
<%}%>
</table>

</div>
<%
  }
  page_menu_begin(out);
  page_menu_button(session, out, "mbDone",   "common.buttons.done",  "smsquiz.done_editing");
  page_menu_button(session, out, "mbCancel",   "common.buttons.cancel",  "smsquiz.cancel");
  page_menu_space(out);
  page_menu_end(out);

%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>