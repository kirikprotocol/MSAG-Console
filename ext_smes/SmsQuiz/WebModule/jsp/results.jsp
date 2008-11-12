<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, ru.novosoft.smsc.util.Functions"%>
<%@ page import="mobi.eyeline.smsquiz.beans.Results"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper"%>
<jsp:useBean id="bean" scope="request" class="mobi.eyeline.smsquiz.beans.Results" />

<jsp:setProperty name="bean" property="*"/>

<%

  bean.getTableHelper().processRequest(request);
  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	TITLE = getLocString("smsquiz.title");

  int rowN = 0;
  int beanResult = bean.process(request);
  if (beanResult == Results.RESULT_EXPORT_ALL) {
    if(session.getAttribute("Export results done")==null) {
      session.setAttribute("Export results done",Boolean.TRUE);
      bean.exportAll(response, out);
      return;
    }
    else {
      session.setAttribute("Export results done",null);
      beanResult = Results.RESULT_OK;
    }
  } else {
      session.setAttribute("Export results done",null);
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<%Collection allQuizes = bean.getAllQuizes();
  if ((allQuizes!=null)&&(allQuizes.size() > 0)) {%>
<table class=properties_list>
<col width="10%">
<col width="40%">
<col width="10%">
<col width="40%">
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%= getLocString("smsquiz.label.quiz")%></th>
  <td><select name=quizId><%
  for (Iterator i = allQuizes.iterator(); i.hasNext();) {
    String quizId = (String) i.next();
    String quizIdEnc = StringEncoderDecoder.encode(quizId);
  %><option value="<%=quizIdEnc%>" <%= (bean.isQuizId(quizIdEnc)) ? "selected":""%>><%=StringEncoderDecoder.encode(quizIdEnc)%></option>
<%}%></select></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%= getLocString("smsquiz.label.for_abonent")%></th>
  <td><input class=txt type="text" id="address" name="address"  value="<%=bean.getAddress()%>" size=25 maxlength=25></td>
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
  </select></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "smsquiz.query_results");
page_menu_space(out);
page_menu_end(out);
  }
  else {
  %><div style="color:blue"><%=getLocString("smsquiz.message.no.quiz")%></div><%
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