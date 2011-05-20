<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.region.Region"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper" %>
<%@ page import="ru.sibinco.smsx.stats.beans.SmsxDailyBean" %>
<%@ page import="java.net.URLEncoder" %>
<%@ page import="java.util.Collection" %>
<%@ page import="java.util.Iterator" %>
<%@ page import="java.util.Set" %>
<jsp:useBean id="bean" scope="request" class="ru.sibinco.smsx.stats.beans.SmsxDailyBean" />
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
    case SmsxDailyBean.RESULT_BACK:
      response.sendRedirect("index.jsp");
      return;
    case SmsxDailyBean.RESULT_DOWNLOAD:
      response.sendRedirect(CPATH+"/esme_smsx/smsx_web_daily_csv.jsp?requestId="+requestId+"&region=" + (bean.getRegion() == null ? "" :URLEncoder.encode(bean.getRegion())) +
          "&msc="+(bean.getMsc() == null ? "" : bean.getMsc()));
      return;
	}
%>
<%@ include file="smsx_inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="smsx_inc/header.jsp"%>

<div class=content>

  <input type="hidden" name="requestId" value="<%=requestId%>"/>

<div class=page_subtitle><%=getLocString("smsx.results.webDaily.title")+" ["+bean.getFrom()+'-'+bean.getTill()+']'%></div>

  <table class=properties_list style="padding-top:10px">
    <col width="5%">
    <col width="15%">
    <col width="5%">
    <col width="15%">
    <col width="40%">
    <tr class=row<%=rowN++&1%>><td style="text-align:left"><%=getLocString("smsx.stat.region")%></td>
    <td>
      <%String region = bean.getRegion();%>
      <select name="region">
        <option value=""><%=getLocString("smsx.stat.region.ALL")%></option>
        <option value="Unknown" <%="Unknown".equals(region)? "SELECTED": ""%>><%=getLocString("smsx.stat.region.unknown")%></option>
        <%Collection regions = bean.getRegions();
          Iterator it = regions.iterator();
        while(it.hasNext()) {
          String r = (String)it.next();
          if(r.equals("Unknown")) {
            continue;
          }%>
        <option value="<%=r%>" <%=r.equals(region)? "SELECTED": ""%>><%=r%></option>
        <%}%>
      </select>
    </td>
    <td style="text-align:left"><%=getLocString("smsx.stat.trafficType")%></td>
    <td>
      <%String msc = bean.getMsc();%>
      <select name="msc">
        <option value=""><%=getLocString("smsx.stat.msc.ALL")%></option>
        <option value="<%=true%>" <%="true".equals(msc)? "SELECTED": ""%>><%=getLocString("smsx.traffic.paid")%></option>
        <option value="<%=false%>" <%="false".equals(msc)? "SELECTED": ""%>><%=getLocString("smsx.traffic.free")%></option>
      </select>
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