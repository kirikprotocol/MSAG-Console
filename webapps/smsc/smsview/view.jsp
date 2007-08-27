<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.admin.smsview.SmsRow,
                 ru.novosoft.smsc.jsp.smsview.SmsViewFormBean,
                 java.text.SimpleDateFormat,
                 java.util.Enumeration,
                 java.util.Hashtable" %>
<jsp:useBean id="smsViewBean" scope="session" class="ru.novosoft.smsc.jsp.smsview.SmsViewFormBean"/>
<% SmsViewFormBean bean = smsViewBean; %>
<jsp:setProperty name="smsViewBean" property="mbView"/>
<jsp:setProperty name="smsViewBean" property="viewId"/>
<%
    TITLE = getLocString("smsview.viewDetailTitle");

    int beanResult = bean.process(request, response);
    
    switch (beanResult) {
        case SmsViewFormBean.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case SmsViewFormBean.RESULT_FILTER:
        case SmsViewFormBean.RESULT_OK:
            break;
        case SmsViewFormBean.RESULT_ERROR:
            break;
        default:
            errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<div class=content>
<input type=hidden name=viewId>
<%
    SmsRow row = bean.getRow();
    if (row != null) {
        SimpleDateFormat dateFormatter = new SimpleDateFormat("dd MMMM, yyyy HH:mm:ss", getLoc());
        int rowN = 0;
        Hashtable ht = row.getBodyParameters();
%>
<div class=page_subtitle>SMS #<%= row.getId()%></div>
<table class=properties_list cellspacing=0>
    <tbody>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.originAddress")%></th>
            <td nowrap valign=top>
                <% String oaMask = row.getOriginatingAddressMask().getMask();%>
                <%= (oaMask != null && oaMask.length() > 0) ? StringEncoderDecoder.encode(oaMask) : "&nbsp;"%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.destAddress")%></th>
            <td nowrap valign=top>
                <% String daMask = row.getDestinationAddressMask().getMask();%>
                <%= (daMask != null && daMask.length() > 0) ? StringEncoderDecoder.encode(daMask) : "&nbsp;"%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.dealiasedDestAddress")%></th>
            <td nowrap valign=top>
                <% String ddaMask = row.getDealiasedDestinationAddressMask().getMask();%>
                <%= (ddaMask != null && ddaMask.length() > 0) ? StringEncoderDecoder.encode(ddaMask) : "&nbsp;"%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.sourceSmeId")%></th>
            <td nowrap valign=top>
                <% String srcSmeId = row.getSrcSmeId();%>
                <%= (srcSmeId != null && srcSmeId.length() > 0) ? StringEncoderDecoder.encode(srcSmeId) : "&nbsp;"%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.destSmeId")%></th>
            <td nowrap valign=top>
                <% String dstSmeId = row.getDstSmeId();%>
                <%= (dstSmeId != null && dstSmeId.length() > 0) ? StringEncoderDecoder.encode(dstSmeId) : "&nbsp;"%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.routeId")%></th>
            <td nowrap valign=top>
                <% String routeId = row.getRouteId();%>
                <%= (routeId != null && routeId.length() > 0) ? StringEncoderDecoder.encode(routeId) : "&nbsp;"%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.smsStatus")%></th>
            <td nowrap valign=top>
                <%= StringEncoderDecoder.encode(row.getStatus())%> (<%=row.getLastResult()%>)
                <%
                    String errMessage = getLocString("smsc.errcode." + row.getLastResult());
                    if (errMessage == null) errMessage = getLocString("smsc.errcode.unknown");
                %>
                <%= StringEncoderDecoder.encode(errMessage == null ? "" : errMessage)%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.submitTime")%></th>
            <td nowrap valign=top>
                <%= (row.getSubmitTime() != null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getSubmitTime())) : "&nbsp;"%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.validTime")%></th>
            <td nowrap valign=top>
                <%= (row.getValidTime() != null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getValidTime())) : "&nbsp;"%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.lastTryTime")%></th>
            <td nowrap valign=top>
                <%= (row.getLastTryTime() != null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getLastTryTime())) : "&nbsp;"%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.nextTryTime")%></th>
            <td nowrap valign=top>
                <%= (row.getNextTryTime() != null) ? StringEncoderDecoder.encode(dateFormatter.format(row.getNextTryTime())) : "&nbsp;"%>
            </td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.destinationDescriptor")%></th>
            <%String imsi = row.getDestinationDescriptor().getImsi();
              if (imsi == null || imsi.length() == 0)
                imsi = "-";
              String msc = row.getDestinationDescriptor().getMsc();
              if (msc == null || msc.length() == 0)
                msc = "-";
              String desc = imsi + " / " + msc;%>
            <td valign=top><%= (row.getText() != null && row.isTextEncoded()) ? desc : StringEncoderDecoder.encode(desc)%>
                &nbsp;</td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.originatingDescriptor")%></th>
            <%String oimsi = row.getOriginatingDescriptor().getImsi();
              if (oimsi == null || oimsi.length() == 0)
                oimsi = "-";
              String omsc = row.getOriginatingDescriptor().getMsc();
              if (omsc == null || omsc.length() == 0)
                omsc = "-";
              String odesc = oimsi + " / " + omsc;%>
            <td valign=top><%= (row.getText() != null && row.isTextEncoded()) ? odesc : StringEncoderDecoder.encode(odesc)%>
                &nbsp;</td>
        </tr>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%=getLocString("smsview.decodedMessage")%></th>
            <td valign=top><%= (row.getText() != null && row.isTextEncoded()) ? row.getText() : StringEncoderDecoder.encode(row.getText())%>
                &nbsp;</td>
        </tr>
    </tbody>
</table>

<div class=page_subtitle><%=getLocString("smsview.viewSubTitle")%></div>
<table class=list cellspacing=0>
    <tbody>
        <%
            Enumeration htke = ht.keys();
            while (htke.hasMoreElements()) {
                String key = (String) htke.nextElement();
                Object value = ht.get(key); %>
        <tr class=row<%=rowN++&1%>0>
            <th width="1%" nowrap valign=top><%
                String keyMsg = getLocString("sms.body.tag." + key);
                if (keyMsg == null) keyMsg = key;
            %><%= keyMsg%></th>
            <td valign=top><%
                String valMsg;
                if (value instanceof String) valMsg = (String) value;
                else if (value instanceof Integer) valMsg = value.toString() + " 0x" + Integer.toHexString(((Integer) value).intValue());
                else if (value instanceof byte[]) {
                    StringBuffer sb = new StringBuffer();
                    byte[] val = (byte[]) value;
                    for (int i = 0; i < val.length; i++) {
                        if (i > 0) {
                            if (i % 16 == 0) sb.append("<br>");
                            else if (i % 8 == 0) sb.append("&nbsp;&nbsp;");
                            else
                                sb.append(' ');
                        }
                        int k = (((int) val[i]) >> 4) & 0xf;
                        sb.append((char) (k < 10 ? k + 48 : k + 55));
                        k = ((int) val[i]) & 0xf;
                        sb.append((char) (k < 10 ? k + 48 : k + 55));
                    }
                    valMsg = sb.toString();
                } else {
                    valMsg = getLocString("smsview.invalidDataType") + ": " + value.getClass().getName();
                }
            %><span style="font-family: Courier;"><%= valMsg%></span></td>
        </tr>
        <%
            }
        %>
    </tbody>
</table>
<br><br><br><br><br><br><br><br><br><br>
<%}%>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>