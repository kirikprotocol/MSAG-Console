<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper"%>
<%@ page import="ru.sibinco.smsx.stats.backend.SmsxRequest" %>
<%@ page import="ru.sibinco.smsx.stats.beans.SmsxRequestsBean" %>
<jsp:useBean id="bean" scope="request" class="ru.sibinco.smsx.stats.beans.SmsxRequestsBean" />
<jsp:setProperty name="bean" property="*"/>
<%

	TITLE=getLocString("smsx.stats.title");
	MENU0_SELECTION = "MENU0_SERVICES";

  int rowN = 0;
	int beanResult = bean.process(request);
	switch(beanResult)
	{
    case SmsxRequestsBean.RESULT_RESULTS:
      SmsxRequest r = bean.getSmsxRequest();
      if(r == null) {
        break;
      }
      String to;
      if(r.getReportTypesFilter() == null || r.getReportTypesFilter().contains(SmsxRequest.ReportType.SMSX_USERS)) {
        to = "smsx_users.jsp";
      }else if(r.getReportTypesFilter().contains(SmsxRequest.ReportType.TRAFFIC)) {
        to = "smsx_traffic.jsp";
      }else if(r.getReportTypesFilter().contains(SmsxRequest.ReportType.WEB_DAILY)) {
        to = "smsx_web_daily.jsp";
      }else {
        to = "smsx_web_regions.jsp";
      }
      to += "?requestId=" + r.getId();
      response.sendRedirect(to);
      return;
    case SmsxRequestsBean.RESULT_ADD:
      response.sendRedirect("smsx_request.jsp");
      return;

    case SmsxRequestsBean.RESULT_DOWNLOAD:
      response.sendRedirect(CPATH+"/esme_smsx/smsx_requests_zip.jsp?selected="+bean.getSelected());
      return;
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>

<div class=content>


<div class=page_subtitle><%=getLocString("smsx.stats.subtitle")%></div>

  <table class=properties_list style="padding-top:10px">
    <col width="5%">
    <col width="95%">
    <tr class=row<%=rowN++&1%>>
    <td style="text-align:left"><%=getLocString("smsx.stat.state")%></td>
    <td>
      <%String status = bean.getStatus();%>
      <select name="status" onchange="this.form.submit();">
        <option value=""><%=getLocString("smsx.stat.request.state.ALL")%></option>
        <%ru.sibinco.smsx.stats.backend.SmsxRequest.Status[] statuses = bean.getRequestStatuses();
        for(int i =0; i<statuses.length; i++) {%>
        <option value="<%=statuses[i].toString()%>" <%=statuses[i].toString().equals(status)? "SELECTED": ""%>><%=getLocString("smsx.stat.request.state."+statuses[i].toString())%></option>
        <%}%>
      </select>
    </td>
  </tr>
</table>
</div>


<div class="content">

<%
    final PagedStaticTableHelper tableHelper = bean.getTableHelper();
%>
<%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>

</div>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbAdd",    "smsx.buttons.newRequest",    "smsx.buttons.newRequest.hint");
  page_menu_confirm_button(session, out, "mbDelete", "smsx.buttons.deleteSelected", "smsx.buttons.deleteSelected.hint", getLocString("smsx.buttons.deleteSelected.confirm"));
  page_menu_space(out);
  page_menu_end(out);
%>

<script type="text/javascript">
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>