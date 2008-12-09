<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, ru.novosoft.smsc.util.Functions"%>
<%@ page import="ru.novosoft.smsc.jsp.util.tables.QueryResultSet"%>
<%@ page import="mobi.eyeline.smsquiz.replystats.ReplyDataItem"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper"%>
<%@ page import="mobi.eyeline.smsquiz.beans.Replies"%>
<jsp:useBean id="smsQuizRepliesBean" scope="session" class="mobi.eyeline.smsquiz.beans.Replies" />
<% smsQuizRepliesBean.clean(); %>
<jsp:setProperty name="smsQuizRepliesBean" property="*"/>


<%
  Replies bean = smsQuizRepliesBean;
  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
  TITLE = getLocString("smsquiz.title")+": "+getLocString("smsquiz.label.replies");

  int rowN = 0;
  int beanResult = bean.process(request);

  if (beanResult == Replies.RESULT_EXPORT_ALL) {
    if(session.getAttribute("Export replies done")==null) {
      session.setAttribute("Export replies done",Boolean.TRUE);
      bean.exportAll(response, out);
      return;
    }
    else{
      beanResult = Replies.RESULT_OK;
      session.setAttribute("Export replies done",null);
    }
  } else {
      session.setAttribute("Export replies done",null);
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=content>
<%Map allQuizes = bean.getAllQuizes();
  if (allQuizes.size() > 0) {%>
<table class=properties_list>
<col width="10%">
<col width="40%">
<col width="10%">
<col width="40%">
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left">Quiz:</th>
  <td><select name=quizId><%
  for (Iterator i = allQuizes.entrySet().iterator(); i.hasNext();) {
    Map.Entry e = (Map.Entry)i.next();
    String quizId = (String) e.getKey();
    String quizIdEnc = StringEncoderDecoder.encode(quizId);
    String quizName = (String)e.getValue();
    String quizNameEnc = StringEncoderDecoder.encode(quizName);
  %><option value="<%=quizIdEnc%>" <%= (bean.isQuizId(quizId)) ? "selected":""%>><%=quizNameEnc%></option>
<%}%></select></td>
  <th style="text-align:left"><%= getLocString("smsquiz.label.for_abonent")%></th>
  <td><input class=txt type="text" id="address" name="address"  value="<%=bean.getAddress()%>" size=25 maxlength=25></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%= getLocString("smsquiz.label.from_date")%></th>
  <td><input class=calendarField id=fromDate name=fromDate value="<%=StringEncoderDecoder.encode(bean.getFromDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
  <th style="text-align:left"><%= getLocString("smsquiz.label.till_date")%></th>
  <td><input class=calendarField id=tillDate name=tillDate value="<%=StringEncoderDecoder.encode(bean.getTillDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%= getLocString("smsquiz.label.page_size")%></th>
  <td><select name=pageSize>
    <option value=  "5" <%= (bean.getPageSizeInt() ==   5) ? "selected":""%>  >5</option>
    <option value= "10" <%= (bean.getPageSizeInt() ==  10) ? "selected":""%> >10</option>
    <option value= "15" <%= (bean.getPageSizeInt() ==  15) ? "selected":""%> >15</option>
    <option value= "20" <%= (bean.getPageSizeInt() ==  20) ? "selected":""%> >20</option>
    <option value= "25" <%= (bean.getPageSizeInt() ==  25) ? "selected":""%> >25</option>
    <option value= "50" <%= (bean.getPageSizeInt() ==  50) ? "selected":""%> >50</option>
    <option value="100" <%= (bean.getPageSizeInt() == 100) ? "selected":""%>>100</option>
  </select></td><td></td><td></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "smsquiz.query_replies");
page_menu_space(out);
page_menu_end(out);
  }
  else {
%><div style="color: blue"><%=getLocString("smsquiz.message.no.quiz")%></div> <%
}
if (bean.isInitialized()) {
%>
<div class=content>
  <%{final PagedStaticTableHelper tableHelper = bean.getTableHelper();%>
  <%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>
  <%}%>
</div>
<%
page_menu_begin(out);
page_menu_space(out);
page_menu_button(session, out, "mbExportAll", "smsquiz.button.export_all", "smsquiz.export_all");
page_menu_end(out);
}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>