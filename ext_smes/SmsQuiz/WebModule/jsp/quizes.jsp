<%@ include file="/WEB-INF/inc/code_header.jsp"%>

<%@ page import="ru.novosoft.smsc.jsp.*"%>
<%@ page import="mobi.eyeline.smsquiz.beans.QuizesList"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper"%>
<%@ page import="ru.novosoft.smsc.util.Functions"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.smsquiz.beans.QuizesList" />
<jsp:setProperty name="bean" property="*"/>
<%

  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
  bean.getTableHelper().processRequest(request);
	TITLE=getLocString("smsquiz.title")+": "+getLocString("smsquiz.label.quizes");
	MENU0_SELECTION = "MENU0_SERVICES";

	int beanResult = bean.process(request);
  switch (beanResult) {
      case PageBean.RESULT_OK :
      case PageBean.RESULT_ERROR :
      case PageBean.RESULT_DONE:
          break;
      case QuizesList.RESULT_ADD :
          response.sendRedirect("quizAdd.jsp");
          return;
      case QuizesList.RESULT_VIEW :
          response.sendRedirect("quizView.jsp?quizId=" + URLEncoder.encode(bean.getSelectedQuizId()));
          return;
      default :
          errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=content>
  <table class=properties_list>
    <col width="10%">
    <col width="40%">
    <col width="10%">
    <col width="40%">
 <% int rowN = 0;   %>
    <tr class=row<%=rowN++&1%>>
      <th style="text-align:left"><%= getLocString("smsquiz.label.from_date")%></th>
      <td><input class=calendarField id=fromDate name=startDate value="<%=StringEncoderDecoder.encode(bean.getStartDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
      <th style="text-align:left"><%= getLocString("smsquiz.label.till_date")%></th>
      <td><input class=calendarField id=tillDate name=tillDate value="<%=StringEncoderDecoder.encode(bean.getTillDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
    </tr>
    <tr>
      <th style="text-align:left"><%= getLocString("smsquiz.label.quiz_name_prefix")%></th>
      <td><input class=txt type="text" name="prefix"  value="<%=bean.getPrefix()%>" size=25 maxlength=25></td>
      <th><%= getLocString("smsquiz.quiz.state")%></th>
    <td><select name=state>
      <option value="" <%= (bean.isState("")) ? "selected":""%>>All</option>
      <%
      for (Iterator i = bean.getStateStringList().iterator(); i.hasNext();) {
        String state = (String)i.next();
        String stateEnc = StringEncoderDecoder.encode(state);
      %><option value="<%=state%>" <%= (bean.isState(state)) ? "selected":""%>><%=stateEnc%></option>
<%}%> </select></td>
    </tr>
  </table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "smsquiz.query_quizes");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>

  <%final PagedStaticTableHelper tableHelper = bean.getTableHelper();
    if(tableHelper.getSize()>0) {%>
  <%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>
    <%}
    else {%>
  <div style="color:blue"><%=getLocString("smsquiz.message.no.quiz")%></div>
    <%}
    %>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbAdd",    "common.buttons.add",    "smsquiz.quiz.add");
  if((tableHelper.getSize()>0)&&(bean.isOnline())) {
    page_menu_confirm_button(session, out, "mbDelete", "common.buttons.delete", "smsquiz.quiz.delete", getLocString("smsquiz.confirm.quiz.delete"));
  }
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>