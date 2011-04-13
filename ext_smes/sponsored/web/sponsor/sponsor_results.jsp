<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper"%>
<%@ page import="ru.sibinco.sponsored.stats.beans.SponsorResultsBean" %>
<%@ page import="java.util.Set" %>
<jsp:useBean id="bean" scope="request" class="ru.sibinco.sponsored.stats.beans.SponsorResultsBean" />
<jsp:setProperty name="bean" property="*"/>
<%

	TITLE=getLocString("smsx.stats.title");
	MENU0_SELECTION = "MENU0_SERVICES";


  int rowN = 0;
	int beanResult = bean.process(request);
  int requestId = bean.getRequestId() == null ? Integer.MIN_VALUE : bean.getRequestId().intValue();
  Set reports = bean.getReports();
	switch(beanResult)
	{
    case SponsorResultsBean.RESULT_BACK:
      response.sendRedirect("index.jsp");
      return;
    case SponsorResultsBean.RESULT_DOWNLOAD:
      response.sendRedirect(CPATH+"/esme_sponsored/sponsor_results_csv.jsp?requestId="+requestId);
      return;
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>

<div class=content>

  <input type="hidden" name="requestId" value="<%=requestId%>"/>

<div class=page_subtitle><%=getLocString("sponsored.results.title")+" ["+bean.getFrom()+'-'+bean.getTill()+']'%></div>

  <table class=properties_list style="padding-top:10px">
    <col width="5%">
    <col width="15%">
    <col width="80%">
    <tr class=row<%=rowN++&1%>>
    <td style="text-align:left"><%=getLocString("sponsored.address")%></td>
    <td><input class="text" name="address" value="<%=bean.getAddress()%>">
    </td>
    <td>&nbsp;</td>
  </tr>
</table>
  <%
  page_menu_begin(out);
  page_menu_button(session, out, "mbQuery", "common.buttons.queryExcl", "common.buttons.runQuery");
  page_menu_space(out);
  page_menu_end(out);
    %>
</div>


<div class="content">

<%
    final PagedStaticTableHelper tableHelper = bean.getTableHelper();
%>
<%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>

</div>

<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbBack",    "smsx.buttons.backToRequest",    "smsx.buttons.backToRequest");
  page_menu_space(out);
  page_menu_button(session, out, "mbDownload",    "smsx.stat.request.download",    "smsx.stat.request.download");
  page_menu_end(out);
%>

<script type="text/javascript">
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>