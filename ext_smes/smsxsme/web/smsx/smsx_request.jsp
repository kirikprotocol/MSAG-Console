<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.sibinco.smsx.stats.beans.CreateSmsxRequestBean" %>
<%@ page import="ru.sibinco.smsx.stats.backend.SmsxRequest" %>
<jsp:useBean id="bean" scope="request" class="ru.sibinco.smsx.stats.beans.CreateSmsxRequestBean" />
<jsp:setProperty name="bean" property="*"/>

<%
  TITLE=getLocString("smsx.stats.request.create.title");
  MENU0_SELECTION = "MENU0_SERVICES";

  int beanResult = bean.process(request);
  switch(beanResult)
  {
    case CreateSmsxRequestBean.RESULT_SAVE:
    case CreateSmsxRequestBean.RESULT_CANCEL:
      response.sendRedirect("index.jsp");
      return;
    default:
  }
  int rowN = 0;
%>

<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>

<div class=content>

  <table class=properties_list>
    <col width="30%">
    <col width="70%">
    <tr>
      <td>
        <div class=page_subtitle><%=getLocString("smsx.stats.period")%></div>
        <table class=properties_list>
          <col width="20%">
          <col width="40%">
          <col width="40%">
          <tr>
            <th><%=getLocString("smsx.stat.from")%></th>
            <td><input type=text id="from" name="from" class=calendarField
                       value="<%=bean.getFrom()%>" maxlength=10 style="z-index:22;"><button
                class=calendarButton type=button onclick="return showCalendar(from, false, false);">...</button></td>
            <td>&nbsp;</td>
          </tr>
          <tr>
            <th><%=getLocString("smsx.stat.till")%></th>
            <td><input type=text id="till" name="till" class=calendarField
                       value="<%=bean.getTill()%>" maxlength=10 style="z-index:22;"><button
                class=calendarButton type=button onclick="return showCalendar(till, false, false);">...</button></td>
            <td>&nbsp;</td>
          </tr>
        </table>
      </td>
      <td rowspan="2">
        <div class=page_subtitle><%=getLocString("smsx.stats.services")%></div>

        <table>
          <col width="1%"><col width="99%">
          <tr>
            <td  style="border:none;"><input class="check" id="all_services" type="checkbox" onchange="
                    var b = this.checked;
                    var els = document.getElementsByName('serviceId');
                    for(var i=0;i<els.length;i++) {
                        els[i].checked = b;
                    }" ></td><td style="border:none;"><label for='all_services'><%=getLocString("smsx.stat.service.ALL")%></label></td>
          </tr>

          <%String[] services = bean.getAllServices();
            for(int i=0;i<services.length;i++){
              String service = services[i];%>
          <tr>
            <td style="border:none;"><input class=check type=checkbox name=serviceId id='serviceId_<%=service%>'
                                            onchange="
                                            if(!this.checked)
                                              document.getElementById('all_services').checked=false;
                                            "
                                            value='<%=service%>' <%=bean.isServiceIsChecked(service) ? "checked" : ""%>>
                                            </td><td style="border:none;"><label for='serviceId_<%=service%>'><%=getLocString("smsx.service."+service)%></label></td>
          </tr>
          <%} %>
        </table>
      </td>
    </tr>
    <tr>
      <td>
        <div class=page_subtitle><%=getLocString("smsx.stats.reports")%></div>
        <table>
          <col width="1%"><col width="48%">
          <col width="1%"><col width="48%">
          <tr>
            <td  style="border:none;"><input class="check" id="all_reports" type="checkbox" onchange="
                    var b = this.checked;
                    var els = document.getElementsByName('reports');
                    for(var i=0;i<els.length;i++) {
                        els[i].checked = b;
                    }" ></td><td style="border:none;"><label for='all_services'><%=getLocString("smsx.request.report.all")%></label></td>
            <td>&nbsp;</td>
            <td>&nbsp;</td>
          </tr>

          <%SmsxRequest.ReportType[] reports = bean.getAllReports();
            int i=0;
            for(;i< reports.length;i++){
              String report = reports[i].toString();
              if(i%2 == 0){%><tr><%}%>
          <td style="border:none;"><input class=check type=checkbox name=reports
                                          onchange="
                                            if(!this.checked)
                                              document.getElementById('all_reports').checked=false;
                                            "
                                          id='reports_<%=report%>' value='<%=report%>' <%=bean.isServiceIsChecked(report) ? "checked" : ""%>></td><td style="border:none;"><label for='reports_<%=report%>'><%=getLocString("smsx.request.report."+ report)%></label></td>
          <%if(i%2 == 1){%></tr><%}
        }
          if(i%2 == 0){%><td style="border:none;"/>&nbsp;</tr><%}%>
        </table>
      </td>
    </tr>
  </table>
</div>


<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbApply", "smsx.stat.request.create", "smsx.stat.request.create");
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>

<script type="text/javascript">
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>