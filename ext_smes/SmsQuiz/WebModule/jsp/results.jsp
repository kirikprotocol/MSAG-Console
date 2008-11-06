<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, ru.novosoft.smsc.util.Functions,
                 java.text.SimpleDateFormat"%>
<%@ page import="mobi.eyeline.smsquiz.beans.Results"%>
<%@ page import="ru.novosoft.smsc.jsp.util.tables.QueryResultSet"%>
<%@ page import="mobi.eyeline.smsquiz.results.ResultDataItem"%>
<jsp:useBean id="bean" scope="request" class="mobi.eyeline.smsquiz.beans.Results" />

<% if (!bean.isProcessed()) {%>
  <jsp:setProperty name="bean" property="*"/>
<% } %>

<%
   ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	 TITLE = getLocString("smsquiz.title");

  int rowN = 0;
  int beanResult = bean.process(request);
  if (beanResult == Results.RESULT_EXPORT_ALL) {
    bean.exportAll(response, out);
    return;
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=startPosition id=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=sort>
<%Collection allQuizes = bean.getAllQuizes();
  if (allQuizes.size() > 0) {%>
<table class=properties_list>
<col width="10%">
<col width="40%">
<col width="10%">
<col width="40%">
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left">Quiz:</th>
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
  <th><%= getLocString("infosme.label.page_size")%></th>
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
<%}%>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "smsquiz.query_results");
page_menu_space(out);
page_menu_end(out);

if (bean.isInitialized()) {
%><div class=content><%
QueryResultSet allResults = bean.getResults();
if (allResults == null || allResults.size() == 0)
{%><span style='color:blue;'><%= getLocString("smsquiz.msg.no_results_matched")%></span><%}
else
{%>
<div class=page_subtitle><%= getLocString("smsquiz.label.query_results_b")%>&nbsp;<%= bean.getTotalSize()%>&nbsp;<%= getLocString("smsquiz.label.query_results_e")%></div>
<table class=list cellspacing=0>
<col width="15%">
<col width="13%">
<col width="22%">
<col width="40%">
<thead><tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%=getLocString("smsquiz.label.abonent")%></th>
  <th style="text-align:left"><%=getLocString("smsquiz.label.deliveryDate")%></th>
  <th style="text-align:left"><%=getLocString("smsquiz.label.replyDate")%></th>
  <th style="text-align:left"><%=getLocString("smsquiz.label.category")%></th>
  <th style="text-align:left"><%=getLocString("smsquiz.label.message")%></th>
</tr></thead>
<tbody><%
  int start = bean.getStartPositionInt();
  int end = start + bean.getPageSizeInt();
  int pos = 0;
  for (Iterator i=allResults.iterator(); i.hasNext() && pos < end; pos++) {
    ResultDataItem result = (ResultDataItem)i.next();
    if (pos >= start) {
    %><tr class=row<%=rowN++&1%>>
        <td><%=StringEncoderDecoder.encode((String)result.getValue("msisdn"))%></td>
        <td nowrap><%=StringEncoderDecoder.encode(bean.convertDateToString((Date)result.getValue("deliveryDate")))%></td>
        <td nowrap><%=StringEncoderDecoder.encode(bean.convertDateToString((Date)result.getValue("replyDate")))%></td>
        <td><%=StringEncoderDecoder.encode((String)result.getValue("category"))%></td>
        <td><%=StringEncoderDecoder.encode((String)result.getValue("message"))%></td>
    </tr>
  <%}
  }%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
</div>
<%
page_menu_begin(out);
page_menu_space(out);
page_menu_button(session, out, "mbExportAll", "smsquiz.button.export_all", "smsquiz.export_all");
page_menu_end(out);
}}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>