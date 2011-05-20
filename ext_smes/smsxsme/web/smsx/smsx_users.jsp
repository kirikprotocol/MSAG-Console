<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.region.Region"%>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper" %>
<%@ page import="ru.sibinco.smsx.stats.beans.SmsxUsersBean" %>
<%@ page import="java.net.URLEncoder" %>
<%@ page import="java.util.Collection" %>
<%@ page import="java.util.Iterator" %>
<%@ page import="java.util.Set" %>
<jsp:useBean id="bean" scope="request" class="ru.sibinco.smsx.stats.beans.SmsxUsersBean" />
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
    case SmsxUsersBean.RESULT_BACK:
      response.sendRedirect("index.jsp");
      return;
    case SmsxUsersBean.RESULT_DOWNLOAD:
      response.sendRedirect(CPATH+"/esme_smsx/smsx_users_csv.jsp?requestId="+requestId+"&region=" + (bean.getRegion() == null ? "" :URLEncoder.encode(bean.getRegion())) +
          "&serviceId=" + (bean.getServiceId() == null ? "" : bean.getServiceId()));
      return;
	}
%>
<%@ include file="smsx_inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<%@ include file="smsx_inc/header.jsp"%>

<div class=content>

  <input type="hidden" name="requestId" value="<%=requestId%>"/>

<div class=page_subtitle><%=getLocString("smsx.results.users.title")+" ["+bean.getFrom()+'-'+bean.getTill()+']'%></div>

  <table class=properties_list style="padding-top:10px">
    <col width="5%">
    <col width="15%">
    <col width="5%">
    <col width="15%">
    <col width="60%">
    <tr class=row<%=rowN++&1%>>
    <td style="text-align:left"><%=getLocString("smsx.stat.service")%></td>
    <td>
      <%String service = bean.getServiceId();%>
      <select name="serviceId">
        <option value=""><%=getLocString("smsx.stat.service.ALL")%></option>
        <%String[] services = bean.getAllServices();
        for(int i = 0; i< services.length; i++) {%>
        <option value="<%=services[i]%>" <%=services[i].equals(service)? "SELECTED": ""%>><%=getLocString("smsx.service."+ services[i])%></option>
        <%}%>
      </select>
    </td><td style="text-align:left"><%=getLocString("smsx.stat.region")%></td>
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