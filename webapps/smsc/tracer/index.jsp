<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.tracer.Index,
                 ru.novosoft.smsc.admin.smsview.SmsQuery,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.tracer.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE = "Routes Tracing";
  switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
  {
    case Index.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case Index.RESULT_OK:
      STATUS.append("Ok");
      break;
    case Index.RESULT_ERROR:
      STATUS.append("<span class=CF00>Error</span>");
      break;
    default:
      STATUS.append("<span class=CF00>Error</span>");
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
  }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
  MENU0_SELECTION = "MENU0_TRACER";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<div class=page_subtitle>Tracing parameters</div>
<table class=properties_list cell>
  <tr class=row0>
    <th>Destination Address:</th>
    <td nowrap><input class=txt type="text" name="dstAddress"
                value="<%=StringEncoderDecoder.encode(bean.getDstAddress())%>" size=25 maxlength=25></td>
  </tr>
  <tr class=row1>
    <th>Source Address:</th>
    <td nowrap><input class=txt type="text" name="srcAddress"
                value="<%=StringEncoderDecoder.encode(bean.getSrcAddress())%>" size=25 maxlength=25></td>
  </tr>
  <tr class=row0>
    <th>Source System Id:</th>
    <td nowrap><input class=txt type="text" name="srcSysId"
                value="<%=StringEncoderDecoder.encode(bean.getSrcSysId())%>" size=25 maxlength=25></td>
  </tr>
</table>
</div><%
  page_menu_begin(out);
  page_menu_button(out, "mbCheck", "Load & Check", "Load & Check routing configuration");
  page_menu_button(out, "mbTrace", "Trace route", "Trace route");
  page_menu_space(out);
  page_menu_end(out);
  String traceResults = bean.getTraceResults();
  if (traceResults != null) {%>
  <div class=content>
  <div class=page_subtitle>Tracing results</div>
    <table class=properties_list cell>
      <tr class=row0><td nowrap><%=StringEncoderDecoder.encode(traceResults)%></td></tr>
    </table>
  </div>
<%}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

