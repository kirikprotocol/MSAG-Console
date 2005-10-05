<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, java.text.SimpleDateFormat,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.PageBean"%>
<%@ page import="ru.novosoft.smsc.admin.smsstat.*"%>
<%@ page import="ru.novosoft.smsc.jsp.smsstat.*"%>
<jsp:useBean id="smsStatFormBean" scope="page" class="ru.novosoft.smsc.jsp.smsstat.SmsStatFormBean" />
<jsp:setProperty name="smsStatFormBean" property="*"/>
<%
    TITLE = getLocString("stat.title");
    MENU0_SELECTION = "MENU0_SMSSTAT";

	SmsStatFormBean bean = smsStatFormBean;
    int beanResult = bean.process(request);
    switch(beanResult)
    {
        case SmsStatFormBean.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case SmsStatFormBean.RESULT_FILTER:
            response.sendRedirect("routsdetail.jsp");
            return;
        case SmsStatFormBean.RESULT_OK:
            break;
        case SmsStatFormBean.RESULT_ERROR:
            break;
        default:
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
    boolean needCSVDownload = (request.getParameter("csv") != null) && (beanResult == PageBean.RESULT_OK);
    if (needCSVDownload) FORM_URI=CPATH+"/smsstat/csv_download.jsp";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<div class=page_subtitle><%=getLocString("stat.subTitle")%></div>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0>
<tr class=row0>
	<th><%=getLocString("common.util.FromDate")%>:</th>
	<td nowrap><input type=text id="fromDate" name="fromDate" class=calendarField value="<%=bean.getFromDate()%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
	<th><%=getLocString("common.util.TillDate")%>:</th>
	<td nowrap><input type=text id="tillDate" name="tillDate" class=calendarField value="<%=bean.getTillDate()%>" maxlength=20><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",
                 (needCSVDownload) ? "common.buttons.statDownload":"common.buttons.statQuery",
                 (needCSVDownload) ? "common.buttons.statDownloadHint":"common.buttons.statQueryHint");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ results ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
Statistics stat = bean.getStatistics();
int disNo = 1;
if (stat != null && !needCSVDownload)
{
    CountersSet total = stat.getTotal();
    Collection dates = stat.getDateStat();
%>
<script language="JavaScript">
  function toggleVisible(p, c)
  {
    var o = p.className == "collapsing_list_opened";
    p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
    c.runtimeStyle.display = o ? "none" : "block";
  }
</script>
<table class=list cellspacing=0>
<tr>
    <td colspan=8><div class=page_subtitle><%=getLocString("stat.generalSubTitle")%></div></td>
</tr>
<tr class=row0>
    <th width="23%"><div align=right>&nbsp;</div></th>
    <th width="11%"><div align=right><%=getLocString("stat.accepted")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.rejected")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.delivered")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.failed")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.rescheduled")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.temporal")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.peakInOut")%></div></th>
</tr>
<tr class=row1>
    <td width="23%" align=right><%=getLocString("stat.totalSmsProcessed")%>:</td>
    <td width="11%" align=right><%= total.accepted%></td>
    <td width="11%" align=right><%= total.rejected%></td>
    <td width="11%" align=right><%= total.delivered%></td>
    <td width="11%" align=right><%= total.failed%></td>
    <td width="11%" align=right><%= total.rescheduled%></td>
    <td width="11%" align=right><%= total.temporal%></td>
    <td width="11%" align=right><%= total.peak_i%>&nbsp;/&nbsp;<%= total.peak_o%></td>
</tr>
<%
Iterator i = dates.iterator();
SimpleDateFormat formatter = new SimpleDateFormat("dd-MMM-yyyy", getLoc());
while (i.hasNext())
{
    Object obj = i.next();
    DateCountersSet date = (DateCountersSet)obj;
    String dateStr = formatter.format(date.getDate());
%>  <tr class=row0>
        <td width="23%" align=right style="cursor:hand" onClick="toggleVisible(opForm.all.p<%=disNo%>, opForm.all.c<%=disNo%>);"><div id="p<%=disNo%>" class=collapsing_list_<%=i.hasNext() ? "closed" : "opened"%>><%= dateStr%><div></td>
        <td width="11%" align=right><%= date.accepted%></td>
        <td width="11%" align=right><%= date.rejected%></td>
        <td width="11%" align=right><%= date.delivered%></td>
        <td width="11%" align=right><%= date.failed%></td>
        <td width="11%" align=right><%= date.rescheduled%></td>
        <td width="11%" align=right><%= date.temporal%></td>
        <td width="11%" align=right><%= date.peak_i%>&nbsp;/&nbsp;<%= date.peak_o%></td>
    </tr>
    <tr id="c<%=disNo++%>" style="display:<%=i.hasNext() ? "none" : "block"%>">
      <td colspan=8><table class=list cellspacing=0 cellpadding=0 border=0>
    <%
    Collection hours = date.getHourStat();
    Iterator j = hours.iterator();
    while (j.hasNext()) {
        HourCountersSet hour = (HourCountersSet)j.next();
    %>  <tr class=row1>
            <td width="23%" align=right>&nbsp;&nbsp;&nbsp;<%= ((hour.getHour() < 10)? "0"+hour.getHour():""+hour.getHour())%></td>
            <td width="11%" align=right><%= hour.accepted%></td>
            <td width="11%" align=right><%= hour.rejected%></td>
            <td width="11%" align=right><%= hour.delivered%></td>
            <td width="11%" align=right><%= hour.failed%></td>
            <td width="11%" align=right><%= hour.rescheduled%></td>
            <td width="11%" align=right><%= hour.temporal%></td>
            <td width="11%" align=right><%= hour.peak_i%>&nbsp;/&nbsp;<%= hour.peak_o%></td>
        </tr><%
    }%>
    </table></td>
    </tr>
<%
}
    Collection smeids = stat.getSmeIdStat();
    i = smeids.iterator();
    if (i.hasNext()) {
%>
<tr>
    <td colspan=8><div class=page_subtitle><%=getLocString("stat.smeActSubTitle")%></div></td>
</tr>
<tr class=row0>
    <th width="23%"><div align=right><%=getLocString("smsview.smeId")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.accepted")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.rejected")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.delivered")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.failed")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.rescheduled")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.temporal")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.peakInOut")%></div></th>
</tr><%
        while (i.hasNext()) {
            SmeIdCountersSet sme = (SmeIdCountersSet)i.next();
            String smeStr = StringEncoderDecoder.encode(sme.smeid);
            Collection errs = sme.getErrors();
        %>
        <tr class=row0>
            <td width="23%" align=right style="cursor:hand" onClick="toggleVisible(opForm.all.p<%=disNo%>, opForm.all.c<%=disNo%>);"><div id="p<%=disNo%>" class=collapsing_list_closed><%= smeStr%><div></td>
            <td width="11%" align=right><%= sme.accepted%></td>
            <td width="11%" align=right><%= sme.rejected%></td>
            <td width="11%" align=right><%= sme.delivered%></td>
            <td width="11%" align=right><%= sme.failed%></td>
            <td width="11%" align=right><%= sme.rescheduled%></td>
            <td width="11%" align=right><%= sme.temporal%></td>
            <td width="11%" align=right><%= sme.peak_i%>&nbsp;/&nbsp;<%= sme.peak_o%></td>
        </tr>
        <tr id="c<%=disNo++%>" style="display:none">
        <td colspan=8><table class=list cellspacing=0 cellpadding=0 border=0>
        <%
          Iterator si = errs.iterator();
          while (si.hasNext()) {
            ErrorCounterSet errid = (ErrorCounterSet)si.next();
          %>
          <tr class=row1>
              <td width="23%" align=right nowrap>
          <%  String errMessage = getLocString("smsc.errcode."+errid.errcode);
              if (errMessage == null) errMessage = getLocString("smsc.errcode.unknown"); %>
          <%= StringEncoderDecoder.encode(errMessage == null ? "" : errMessage)%>
          (<%=errid.errcode%>)
              </td>
              <td width="11%" align=right><%= errid.counter%></td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
          </tr><%
          }
        %>
        </table></td>
        </tr><%
        }
    }

    Collection routeids = stat.getRouteIdStat();
    i = routeids.iterator();
    boolean needRoutes = i.hasNext();
    if (needRoutes) {
%>
<tr>
    <td colspan=8> <div class=page_subtitle><%=getLocString("stat.routSubTitle")%></div></td>
</tr>
<tr colspan=8 class=row0>
    <th width="23%"><div align=right><%=getLocString("smsview.routeId")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.accepted")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.rejected")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.delivered")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.failed")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.rescheduled")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.temporal")%></div></th>
    <th width="11%"><div align=right><%=getLocString("stat.peakInOut")%></div></th>
</tr>  <%
        while (i.hasNext()) {
            RouteIdCountersSet route = (RouteIdCountersSet)i.next();
            String routeStr = StringEncoderDecoder.encode(route.routeid);
            Collection errs = route.getErrors();
        %>
        <tr class=row0>
            <td width="23%" align=right style="cursor:hand" onClick="toggleVisible(opForm.all.p<%=disNo%>, opForm.all.c<%=disNo%>);"><div id="p<%=disNo%>" class=collapsing_list_closed><%= routeStr%><div></td>
            <td width="11%" align=right><%= route.accepted%></td>
            <td width="11%" align=right><%= route.rejected%></td>
            <td width="11%" align=right><%= route.delivered%></td>
            <td width="11%" align=right><%= route.failed%></td>
            <td width="11%" align=right><%= route.rescheduled%></td>
            <td width="11%" align=right><%= route.temporal%></td>
            <td width="11%" align=right><%= route.peak_i%>&nbsp;/&nbsp;<%= route.peak_o%></td>
        </tr>
        <tr id="c<%=disNo++%>" style="display:none">
        <td colspan=8><table class=list cellspacing=0 cellpadding=0 border=0>
        <%
          Iterator ri = errs.iterator();
          while (ri.hasNext()) {
            ErrorCounterSet errid = (ErrorCounterSet)ri.next();
          %>
          <tr class=row1>
              <td width="23%" align=right nowrap>
          <%  String errMessage = getLocString("smsc.errcode."+errid.errcode);
              if (errMessage == null) errMessage = getLocString("smsc.errcode.unknown"); %>
          <%= StringEncoderDecoder.encode(errMessage == null ? "" : errMessage)%>
          (<%=errid.errcode%>)
              </td>
              <td width="11%" align=right><%= errid.counter%></td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
              <td width="11%" align=right>&nbsp;</td>
          </tr><%
          }
        %>
        </table></td>
        </tr><%
        }
    }
%>
</table>
</div>
<% if (needRoutes) {
page_menu_begin(out);
page_menu_button(session, out, "mbDetail",  "stat.routDetails", "common.buttons.routDetailsHint");
page_menu_space(out);
page_menu_end(out);
} %>
<div class=content>
<%
    Collection errids = stat.getErrorStat();
    i = errids.iterator();
    if (i.hasNext()) {
%>
<table class=list cellspacing=0>
<tr>
    <td colspan=8><div class=page_subtitle><%=getLocString("stat.smsSubTitle")%></div></td>
</tr>
<tr class=row0>
    <th width="23%"><div align=right><%=getLocString("stat.errorCode")%></div></th>
    <th width="11%"><div align=right><%=getLocString("common.util.Count")%></div></th>
    <th width="11%">&nbsp;</th>
    <th width="11%">&nbsp;</th>
    <th width="11%">&nbsp;</th>
    <th width="11%">&nbsp;</th>
    <th width="11%">&nbsp;</th>
    <th width="11%">&nbsp;</th>
</tr>  <%
        while (i.hasNext()) {
          ErrorCounterSet errid = (ErrorCounterSet)i.next();
        %>
        <tr class=row1>
            <td width="23%" align=right nowrap>
        <% String errMessage = getLocString("smsc.errcode."+errid.errcode);
           if (errMessage == null) errMessage = getLocString("smsc.errcode.unknown"); %>
        <%= StringEncoderDecoder.encode(errMessage == null ? "" : errMessage)%>
        (<%=errid.errcode%>)
            </td>
            <td width="11%" align=right><%= errid.counter%></td>
            <td width="11%" align=right>&nbsp;</td>
            <td width="11%" align=right>&nbsp;</td>
            <td width="11%" align=right>&nbsp;</td>
            <td width="11%" align=right>&nbsp;</td>
            <td width="11%" align=right>&nbsp;</td>
            <td width="11%" align=right>&nbsp;</td>
        </tr><%
        }%>
</table>
<%
    }
} // stat != null %>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

