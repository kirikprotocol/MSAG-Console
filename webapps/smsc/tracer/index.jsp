<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.tracer.Index,
                 ru.novosoft.smsc.admin.smsview.SmsQuery,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.List,
                 java.util.Hashtable,
                 java.util.Enumeration,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.tracer.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE = "Routes Tracing";
  switch(bean.process(request))
  {
    case Index.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case Index.RESULT_OK:

      break;
    case Index.RESULT_ERROR:

      break;
    default:

      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
  MENU0_SELECTION = "MENU0_TRACER";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%if (bean.getAppContext().getStatuses().isRoutesLoaded()) {
%>
<div class=content>
<div class=page_subtitle>Tracing parameters</div>
<table class=properties_list cell>
  <tr class=row1>
    <th width="30%">Source Address:</th>
    <td width="70%" nowrap><input class=txt type="text" name="srcAddress"
        value="<%=StringEncoderDecoder.encode(bean.getSrcAddress())%>" validation="mask" size=25 maxlength=25></td>
  </tr>
  <tr class=row0>
    <th width="30%">Destination Address:</th>
    <td width="70%" nowrap><input class=txt type="text" name="dstAddress"
        value="<%=StringEncoderDecoder.encode(bean.getDstAddress())%>" validation="mask" size=25 maxlength=25></td>
  </tr>
  <tr class=row0>
    <th width="30%">Source System Id:</th>
    <td width="70%" nowrap><input class=txt type="text" name="srcSysId"
        value="<%=StringEncoderDecoder.encode(bean.getSrcSysId())%>" size=25 maxlength=25></td>
  </tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbCheck", "Loadup & Check", "Load & Check routing configuration");
page_menu_button(out, "mbTrace", "Trace route", "Trace route");
page_menu_space(out);
page_menu_end(out);
} else { %>
<div class=content>
<table class=properties_list cell>
  <tr class=row0><td>
    <span class="C800"><b>Warning !</b></span><br><br>
    <span class="C000">Active routing configuration is not loaded or has been changed.</span><br>
    <span class="C000">Please loadup new routing information to route tracer.</span><br>&nbsp;
  </td></tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbCheck", "Loadup & Check", "Load & Check routing configuration");
page_menu_space(out);
page_menu_end(out);
}
  String message = bean.getTraceMessage();
  List   traceResults = bean.getTraceResults();
  List   routeInfo = bean.getRouteInfo();
  int rowN = 0;
  if (message != null || traceResults != null || routeInfo != null) {%>
  <div class=content>
<%if (message != null) {
  int mt = bean.getMessageType();%>
  <table class=properties_list cell>
  <tr><td>
  <span class="<%= (mt == bean.TRACE_ROUTE_FOUND) ? "C080":
                   ((mt == bean.TRACE_ROUTE_NOT_FOUND) ? "C800":"C008")%>">
    <b><%=StringEncoderDecoder.encode(message)%></b>
  </span>
  </td></tr>
  </table>
<%}%>
<%if (routeInfo != null) {%>
  <br>
  <div class=page_subtitle>Route info</div>
  <table class=properties_list cell><%
    for (int i=0; i<routeInfo.size(); i+=2)
    {
      String key = (String)routeInfo.get(i);
      String val = (String)routeInfo.get(i+1);%>
      <tr class=row<%=rowN++&1%>>
        <th width="30%" nowrap><%=(key != null && key.trim().length()>0) ? StringEncoderDecoder.encode(key):"&nbsp;"%></th>
        <td width="70%" nowrap><%=(val != null && val.trim().length()>0) ? StringEncoderDecoder.encode(val):"&nbsp;"%></td>
      </tr><%
    }%>
  </table>
<%}%>
<%if (traceResults != null && traceResults.size() > 0) {%>
  <br>
  <div class=page_subtitle>Trace</div>
  <textarea style="font-family:Courier New;height:300px"><%
    for (int i=0; i<traceResults.size(); i++) {
      Object traceObj = traceResults.get(i);
      if (traceObj != null && traceObj instanceof String) { %><%=
          StringEncoderDecoder.encode((String)traceObj)+'\n'%><%
      }
    }
  %></textarea>
<%}%>
  </div>
<%}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

