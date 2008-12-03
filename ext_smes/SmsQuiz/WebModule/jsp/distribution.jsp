<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, ru.novosoft.smsc.infosme.backend.Message"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper"%>
<%@ page import="mobi.eyeline.smsquiz.beans.Distribution"%>
<%@ page import="ru.novosoft.smsc.util.*"%>
<jsp:useBean id="smsQuizDistributionBean" scope="session" class="mobi.eyeline.smsquiz.beans.Distribution" />
<% smsQuizDistributionBean.clean(); %>
<jsp:setProperty name="smsQuizDistributionBean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("smsquiz.title");
  ServiceIDForShowStatus = ru.novosoft.smsc.util.Functions.getServiceId(request.getServletPath());
  MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

  Distribution bean = smsQuizDistributionBean;
  int rowN = 0;
  int beanResult = bean.process(request);
  if (beanResult == Distribution.RESULT_EXPORT_ALL) {
    if(session.getAttribute("Export distr done")==null) {
      session.setAttribute("Export distr done",Boolean.TRUE);
      bean.exportAll(response, out);
      return;
    } else {
      beanResult = Distribution.RESULT_OK;
      session.setAttribute("Export distr done",null);
    }
  } else {
    session.setAttribute("Export distr done",null);
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
  <th style="text-align:left"><%= getLocString("smsquiz.label.quiz")%></th>
  <td><select name=quizId><%
  for (Iterator i = allQuizes.entrySet().iterator(); i.hasNext();) {
    Map.Entry e = (Map.Entry)i.next();
    String quizId = (String) e.getKey();
    String quizIdEnc = StringEncoderDecoder.encode(quizId);
    String quizName = (String)e.getValue();
    String quizNameEnc = StringEncoderDecoder.encode(quizName);
  %><option value="<%=quizIdEnc%>" <%= (bean.isQuizId(quizIdEnc)) ? "selected":""%>><%=quizNameEnc%></option>
<%}%></select></td>
  <th style="text-align:left"><%= getLocString("smsquiz.label.msg_state")%></th>
  <td><select name=status>
        <option value="<%=Message.State.UNDEFINED.getId()%>" <%= (bean.isStatus(Message.State.UNDEFINED.getId())) ? "selected":""%>>ALL</option>
        <option value="<%=Message.State.NEW.getId()%>"       <%= (bean.isStatus(Message.State.NEW.getId())) ? "selected":""%>      >NEW</option>
        <option value="<%=Message.State.WAIT.getId()%>"      <%= (bean.isStatus(Message.State.WAIT.getId())) ? "selected":""%>     >WAIT</option>
        <option value="<%=Message.State.ENROUTE.getId()%>"   <%= (bean.isStatus(Message.State.ENROUTE.getId())) ? "selected":""%>     >ENROUTE</option>
        <option value="<%=Message.State.DELIVERED.getId()%>" <%= (bean.isStatus(Message.State.DELIVERED.getId())) ? "selected":""%>>DELIVERED</option>
        <option value="<%=Message.State.EXPIRED.getId()%>"   <%= (bean.isStatus(Message.State.EXPIRED.getId())) ? "selected":""%>  >EXPIRED</option>
        <option value="<%=Message.State.FAILED.getId()%>"    <%= (bean.isStatus(Message.State.FAILED.getId())) ? "selected":""%>   >FAILED</option>
  </select></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%= getLocString("smsquiz.label.from_date")%></th>
  <td><input class=calendarField id=fromDate name=fromDate value="<%=StringEncoderDecoder.encode(bean.getFromDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
  <th style="text-align:left"><%= getLocString("smsquiz.label.till_date")%></th>
  <td><input class=calendarField id=tillDate name=tillDate value="<%=StringEncoderDecoder.encode(bean.getTillDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%= getLocString("smsquiz.label.for_abonent")%></th>
  <td><input class=txt type="text" id="address" name="address"  value="<%=bean.getAddress()%>" size=25 maxlength=25></td>
  <th><%= getLocString("smsquiz.label.page_size")%></th>
  <td><select name=pageSize>
    <option value=  "5" <%= (bean.getPageSizeInt() ==   5) ? "selected":""%>  >5</option>
    <option value= "10" <%= (bean.getPageSizeInt() ==  10) ? "selected":""%> >10</option>
    <option value= "15" <%= (bean.getPageSizeInt() ==  15) ? "selected":""%> >15</option>
    <option value= "20" <%= (bean.getPageSizeInt() ==  20) ? "selected":""%> >20</option>
    <option value= "25" <%= (bean.getPageSizeInt() ==  25) ? "selected":""%> >25</option>
    <option value= "50" <%= (bean.getPageSizeInt() ==  50) ? "selected":""%> >50</option>
    <option value="100" <%= (bean.getPageSizeInt() == 100) ? "selected":""%>>100</option>
  </select></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "smsquiz.query_messages");
page_menu_space(out);
page_menu_end(out);
} else {
  %>
<div style="color: blue"><%=getLocString("smsquiz.message.no.quiz")%></div>
<%
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
page_menu_button(session, out, "mbExportAll", "smsquiz.button.export_all", "smsquiz.button.export_all");
page_menu_end(out);
}%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>