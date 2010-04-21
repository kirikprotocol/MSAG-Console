<%@ page import="java.util.*"%>
<%@ page import="java.text.SimpleDateFormat"%>
<%@ page import="ru.novosoft.smsc.emailsme.beans.Statistic"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.emailsme.beans.Stats" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Email SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";

  int beanResult = bean.process(request);
  int rowN = 0;
%><%@ include file="switch_menu.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="menu.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<div class=content>
  <div class=page_subtitle><%= getLocString("infosme.subtitle.stat_params")%></div>
  <input type=hidden name=initialized value=true>
  <table class=properties_list>
    <col width="5%">
    <col width="45%">
    <col width="5%">
    <col width="45%">
    <tr class=row<%=rowN++&1%>>
      <th style="text-align:left"><%= getLocString("infosme.label.from_date")%></th>
      <td><input class=calendarField id=from name=from value="<%=StringEncoderDecoder.encode(bean.getFrom())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(from, false, true);">...</button></td>
      <th style="text-align:left"><%= getLocString("infosme.label.till_date")%></th>
      <td><input class=calendarField id=till name=till value="<%=StringEncoderDecoder.encode(bean.getTill())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(till, false, true);">...</button></td>
    </tr>
  </table>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "infosme.hint.query_stat");
  page_menu_space(out);
  page_menu_end(out);
%>
<%
  java.util.Map statistics = bean.getStats();
  if (statistics != null) {%>
<div class=content>
  <script type="text/javascript">
    function toggleVisible(p, c) {
      var o = p.className == "collapsing_list_opened";
      p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
      c.style.display = o ? "none" : "";
    }
  </script>

  <table class=list cellspacing=0>
    <col width="60%">
    <col width="10%">
    <col width="10%">
    <col width="10%">
    <col width="10%">
    <tr>
      <td><%= getLocString("emailsme.label.date")%></td>
      <td align="center"><%= getLocString("emailsme.label.received_ok")%></td>
      <td align="center"><%= getLocString("emailsme.label.received_fail")%></td>
      <td align="center"><%= getLocString("emailsme.label.transmitted_ok")%></td>
      <td align="center"><%= getLocString("emailsme.label.transmitted_fail")%></td>
    </tr>
<%
  ArrayList keys = new ArrayList(statistics.keySet());
  Collections.sort(keys);

  SimpleDateFormat formatter = new SimpleDateFormat("dd-MMM-yyyy");
  for (int i=keys.size()-1; i >= 0; i--) {
    Object key = keys.get(i);
    String date = formatter.format(key);
    String dateHex = StringEncoderDecoder.encodeHEX(date);
    Statistic stat = (Statistic)statistics.get(key);

%>
    <tr class=row0>
      <td><div id="p<%=dateHex%>" class="collapsing_list_<%=i>0 ? "closed" : "opened"%>" onClick="toggleVisible(this, document.getElementById('c<%=dateHex%>'));"><%=date%></div></td>
      <td align="center"><%=stat.getTotalReceivedOk()%></td>
      <td align="center"><%=stat.getTotalReceivedFail()%></td>
      <td align="center"><%=stat.getTotalTransmittedOk()%></td>
      <td align="center"><%=stat.getTotalTransmittedFail()%></td>
    </tr>
    <tr id="c<%=dateHex%>" style="display:<%=i>0 ? "none" : ""%>">
      <td colspan=5>
        <table class=list cellspacing=0 cellpadding=0 border=0>
          <col width="60%">
          <col width="10%">
          <col width="10%">
          <col width="10%">
          <col width="10%">
<%
  for (Iterator j = stat.getStats().iterator(); j.hasNext();) {
    Statistic.StatRecord rec = (Statistic.StatRecord) j.next();%>
          <tr class=row1>
            <td><%=rec.getHour()%></td>
            <td align="center"><%=rec.getReceivedOk()%></td>
            <td align="center"><%=rec.getReceivedFail()%></td>
            <td align="center"><%=rec.getTransmittedOk()%></td>
            <td align="center"><%=rec.getTransmittedFail()%></td>
          </tr>
<%}%>
        </table>
      </td>
    </tr>
<%}%>
  </table>
</div>
<%}%>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
