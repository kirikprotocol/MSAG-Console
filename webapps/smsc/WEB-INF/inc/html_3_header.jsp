<%@ page errorPage="/error.jsp" %>
<%@ page session="true" %>
<%@ include file="/WEB-INF/inc/html_0_header.jsp" %>
<script>
    function openPerfMon() {
        open("<%=CPATH%>/perfmon/index.jsp", "perfmon-<%=request.getServerName()%>", "channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=580");
        return false;
    }
    function openTopMon() {
        open("<%=CPATH%>/topmon/index.jsp", "topmon-<%=request.getServerName()%>", "channelmode=no,directories=no,fullscreen=no,location=no,menubar=no,resizable=yes,scrollbars=no,status=no,titlebar=no,toolbar=no,height=450,width=640");
        return false;
    }
</script>
<% if (isServiceStatusNeeded || (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0))
    {
%>
        <%@ include file="/WEB-INF/inc/service_status.jsp" %><%
    }
%>
<%@ include file="/WEB-INF/inc/html_1_header.jsp" %>
<%@ include file="/WEB-INF/inc/messages.jsp" %>

<%-- SMSC Status --%>
<table cellpadding=0 cellspacing=0 height=30px class=smsc_status ><tr>
    <th background="/images/smsc_17.jpg" nowrap><%=TITLE%></th>
    <td>&nbsp;<%
        if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0)
        {
            if (ServiceIDForShowStatus.equals(Constants.SMSC_SME_ID) && Constants.instType == ResourceGroupConstants.RESOURCEGROUP_TYPE_HA)
            { %>
                <%=smscStatus((SMSCAppContext) request.getAttribute("appContext"))%>
                <%
            }
            else
            {
                    %><%=StringEncoderDecoder.encode(ServiceIDForShowStatus)%>&nbsp;
                    <%=getLocString("grammatic.is")%>&nbsp;
                    <%=serviceStatus((SMSCAppContext) request.getAttribute("appContext"), ServiceIDForShowStatus)%><%
            }
        }%></td>
    <td width=12px background="/images/smsc_19.jpg" style="padding-right:0px;"></td></tr>
</table>

<table cellspacing="0" cellpadding="0" >
<tr>
<td width="12px" background="/images/smsc_24.jpg" ></td>


<%--<table cellspacing="0" cellpadding="0" height="100%" style="position: relative; top:0; left:0; z-index:1;">--%>
<%--   <tr><td width="12px" background="/images/smsc_24.jpg" ></td><td>&nbsp;</td><td width="12px" background="/images/smsc_26.jpg"></td>--%>
<%--   </tr>--%>
<%--</table>--%>
<td>
<form name="opForm" id=opForm action="<%=FORM_URI%>" method="<%=FORM_METHOD%>" enctype="<%=FORM_ENCTYPE%>" onSubmit="return validateForm(this)">
  <input type=hidden ID=jbutton value=jbutton>