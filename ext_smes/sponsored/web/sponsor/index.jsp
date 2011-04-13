<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper"%>
<%@ page import="ru.sibinco.sponsored.stats.backend.SponsoredRequest" %>
<%@ page import="ru.sibinco.sponsored.stats.beans.SponsoredRequestsBean" %>
<jsp:useBean id="bean" scope="request" class="ru.sibinco.sponsored.stats.beans.SponsoredRequestsBean" />
<jsp:setProperty name="bean" property="*"/>
<%

	TITLE=getLocString("sponsor.stats.title");
	MENU0_SELECTION = "MENU0_SERVICES";


  int rowN = 0;
	int beanResult = bean.process(request);
	switch(beanResult)
	{
    case SponsoredRequestsBean.RESULT_RESULTS:
      SponsoredRequest r = bean.getSponsorRequest();
      if(r == null) {
        break;
      }
      response.sendRedirect("sponsor_results.jsp?requestId=" + r.getId());
      return;
    case SponsoredRequestsBean.RESULT_ADD:
      response.sendRedirect("sponsor_request.jsp");
      return;

    case SponsoredRequestsBean.RESULT_DOWNLOAD:
      response.sendRedirect(CPATH+"/esme_sponsored/sponsor_requests_csv.jsp?selected="+bean.getSelected());
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
        <%SponsoredRequest.Status[] statuses = bean.getRequestStatuses();
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