<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.smsview.*,
                 ru.novosoft.smsc.util.*,
                 java.text.*,
                 java.util.Hashtable,
                 java.util.Enumeration"%>
<%@ page import="ru.novosoft.smsc.jsp.smsview.*"%>
<jsp:useBean id="smsViewBean" scope="session" class="ru.novosoft.smsc.jsp.smsview.SmsViewFormBean" />
<%
	SmsViewFormBean bean = smsViewBean;
%>
<jsp:setProperty name="smsViewBean" property="*"/>
<%
TITLE="SMS Detailed View";

int beanResult = SmsViewFormBean.RESULT_OK;
switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case SmsViewFormBean.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case SmsViewFormBean.RESULT_FILTER:
	case SmsViewFormBean.RESULT_OK:
		STATUS.append("Ok");
		break;
	case SmsViewFormBean.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
	default:
		STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<input type=hidden name=storageType>
<input type=hidden name=smsId>
<%
  SmsDetailedRow row = bean.getDetailedRow();
  if (row != null) {
    SimpleDateFormat dateFormatter = new SimpleDateFormat("dd MMMM, yyyy HH:mm:ss");
    int rowN=0;
  %>
<div class=secView>SMS #<%= row.getIdLong()%></div>
<table class=secRep cellspacing=1 width="100%">
<tbody>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Originating Address</th>
    <td nowrap valign=top>
      <%= StringEncoderDecoder.encode(row.getOriginatingAddressMask().getMask())%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Destination Address</th>
    <td nowrap valign=top>
      <%= StringEncoderDecoder.encode(row.getDestinationAddressMask().getMask())%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Dealiased Destination Address</th>
    <td nowrap valign=top>
      <%= StringEncoderDecoder.encode(row.getDealiasedDestinationAddressMask().getMask())%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Source SME Id</th>
    <td nowrap valign=top>
      <%= StringEncoderDecoder.encode(row.getSrcSmeId())%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Destination SME Id</th>
    <td nowrap valign=top>
      <%= StringEncoderDecoder.encode(row.getDstSmeId())%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Route Id</th>
    <td nowrap valign=top>
      <%= StringEncoderDecoder.encode(row.getRouteId())%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Status</th>
    <td nowrap valign=top>
      <%= StringEncoderDecoder.encode(row.getStatus())%> (<%=row.getLastResult()%>)
      <%
         String errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode."+row.getLastResult());
         if (errMessage == null) errMessage = appContext.getLocaleString(request.getLocale(), "smsc.errcode.unknown");
      %>
      <%= StringEncoderDecoder.encode(errMessage)%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Submit Time</th>
    <td nowrap valign=top>
      <%= (row.getSubmitTime()!= null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getSubmitTime())):"&nbsp;"%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Valid Time</th>
    <td nowrap valign=top>
      <%= (row.getValidTime()!= null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getValidTime())):"&nbsp;"%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Last Try Time</th>
    <td nowrap valign=top>
      <%= (row.getLastTryTime()!= null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getLastTryTime())):"&nbsp;"%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Next Try Time</th>
    <td nowrap valign=top>
      <%= (row.getNextTryTime()!= null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getNextTryTime())):"&nbsp;"%>
    </td>
  </tr>
  <tr class=row<%=rowN++&1%>0>
    <th width="1%" nowrap valign=top class=label>Decoded Message</th>
    <td nowrap valign=top>
      <%= (row.getText()!=null&&row.getText().startsWith("&#")?row.getText():StringEncoderDecoder.encode(row.getText()))%>&nbsp;
    </td>
  </tr>
</tbody>
</table>

<div class=secView>SMS Body</div>
<table class=secRep cellspacing=1 width="100%">
<tbody>
  <%
    Hashtable ht = row.getBodyParameters();
    Enumeration htke = ht.keys();
    while (htke.hasMoreElements()) {
      String key = (String)htke.nextElement();
      Object value = ht.get(key); %>
    <tr class=row<%=rowN++&1%>0>
      <th width="1%" nowrap valign=top class=label><%
        String keyMsg = appContext.getLocaleString(request.getLocale(), "sms.body.tag."+key);
        if( keyMsg == null ) keyMsg = key;
      %><%= keyMsg%></th>
      <td valign=top><%
        String valMsg;
        if( value instanceof String ) valMsg = (String)value;
        else if ( value instanceof Integer) valMsg = value.toString();
        else if ( value instanceof byte[]) {
          StringBuffer sb = new StringBuffer();
          byte[] val = (byte[])value;
          for (int i=0; i<val.length; i++) {
            if( i > 0 ) {
              if( i % 16 == 0) sb.append("<br>");
              else if( i % 8 == 0 ) sb.append( "&nbsp;&nbsp;");
              else sb.append( ' ' );
            }
            int k = (((int)val[i])>>4)&0xf;
            sb.append( (char)(k<10?k+48:k+55) );
            k = ((int)val[i])&0xf;
            sb.append( (char)(k<10?k+48:k+55) );
          }
          valMsg = sb.toString();
        } else {
          valMsg = "Invalid data type: "+value.getClass().getName();
        }
      %><span style="font-family: Courier;"><%= valMsg%></span></td>
    </tr>
    <%
    }
  %>
</tbody>
</table>
<%}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>