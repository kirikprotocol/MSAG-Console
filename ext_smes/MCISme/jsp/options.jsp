<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants, ru.novosoft.smsc.mcisme.beans.Options,
                 ru.novosoft.smsc.jsp.SMSCJspException, ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.util.StringEncoderDecoder, java.util.*"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Options" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE="Missed Calls Info SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
  int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<div class=page_subtitle>General options</div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%">
<col width="80%">
<% int rowN = 0;%>
<tr class=row<%=rowN++&1%>>
  <th>address</th>
  <td><input class=txt name=address value="<%=StringEncoderDecoder.encode(bean.getAddress())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Service type</th>
  <td><input class=txt name=svcType value="<%=StringEncoderDecoder.encode(bean.getSvcType())%>" maxlength="5"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Protocol ID</th>
  <td><input class=txt name=protocolId value="<%=StringEncoderDecoder.encode(bean.getProtocolId())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Validity period (days)</th>
  <td><input class=txt name=daysValid value="<%=StringEncoderDecoder.encode(bean.getDaysValid())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Calling mask (regexp)</th>
  <td><input class=txt name=callingMask value="<%=StringEncoderDecoder.encode(bean.getCallingMask())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Called mask (regexp)</th>
  <td><input class=txt name=calledMask value="<%=StringEncoderDecoder.encode(bean.getCalledMask())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Unresponded messages max</th>
  <td><input class=txt name=unrespondedMessagesMax value="<%=StringEncoderDecoder.encode(bean.getUnrespondedMessagesMax())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Unresponded messages sleep</th>
  <td><input class=txt name=unrespondedMessagesSleep value="<%=StringEncoderDecoder.encode(bean.getUnrespondedMessagesSleep())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Response wait time</th>
  <td><input class=txt name=responceWaitTime value="<%=StringEncoderDecoder.encode(bean.getResponceWaitTime())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Receipt wait time</th>
  <td><input class=txt name=receiptWaitTime value="<%=StringEncoderDecoder.encode(bean.getReceiptWaitTime())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Input queue size</th>
  <td><input class=txt name=inputQueueSize value="<%=StringEncoderDecoder.encode(bean.getInputQueueSize())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Output queue size</th>
  <td><input class=txt name=outputQueueSize value="<%=StringEncoderDecoder.encode(bean.getOutputQueueSize())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Max rows per message</th>
  <td><input class=txt name=maxRowsPerMessage value="<%=StringEncoderDecoder.encode(bean.getMaxRowsPerMessage())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=inform>Force inform</label></th>
  <td><input class=check type=checkbox name=forceInform id=inform value=true <%=bean.isForceInform() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=notify>Force notify</label></th>
  <td><input class=check type=checkbox name=forceNotify id=notify value=true <%=bean.isForceNotify() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks thread pool max</th>
  <td><input class=txt name=smppThreadPoolMax value="<%=StringEncoderDecoder.encode(bean.getSmppThreadPoolMax())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks thread pool init</th>
  <td><input class=txt name=smppThreadPoolInit value="<%=StringEncoderDecoder.encode(bean.getSmppThreadPoolInit())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Admin host</th>
  <td><input class=txt name=adminHost value="<%=StringEncoderDecoder.encode(bean.getAdminHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Admin port</th>
  <td><input class=txt name=adminPort value="<%=StringEncoderDecoder.encode(bean.getAdminPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC host</th>
  <td><input class=txt name=smscHost value="<%=StringEncoderDecoder.encode(bean.getSmscHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC port</th>
  <td><input class=txt name=smscPort value="<%=StringEncoderDecoder.encode(bean.getSmscPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC SID</th>
  <td><input class=txt name=smscSid value="<%=StringEncoderDecoder.encode(bean.getSmscSid())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC connection timeout</th>
  <td><input class=txt name=smscTimeout value="<%=StringEncoderDecoder.encode(bean.getSmscTimeout())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC connection password</th>
  <td><input class=txt name=smscPassword value="<%=StringEncoderDecoder.encode(bean.getSmscPassword())%>"></td>
</tr>
</table>
<div class=page_subtitle>Circuits settings</div>
<table class=properties_list cellspacing=0  width="100%" <%rowN=0;%>>
<col width="20%">
<col width="80%">
<tr class=row<%=rowN++&1%>>
  <th>Hardware Selection Number</th>
  <td><input class=txt name=HSN value="<%=StringEncoderDecoder.encode(bean.getHSN())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Span</th>
  <td><input class=txt name=SPN value="<%=StringEncoderDecoder.encode(bean.getSPN())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Timeslot mask (hex)</th>
  <td><input class=txt name=TSM value="<%=StringEncoderDecoder.encode(bean.getTSM())%>"></td>
</tr>
</table>
<div class=page_subtitle>Release responce codes</div>
<table class=properties_list cellspacing=0  width="100%" <%rowN=0;%>>
<col width="39%">
<col width="1%">
<col width="60%">
<tr class=row<%=rowN++&1%>>
  <th>Reason</th>
  <th>Responce code</th>
  <th>Inform</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Busy</th>
  <td><input class=txt name=causeBusy value="<%=StringEncoderDecoder.encode(bean.getCauseBusy())%>" validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informBusy value=true <%=bean.isInformBusy() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>No Reply</th>
  <td><input class=txt name=causeNoReply value="<%=StringEncoderDecoder.encode(bean.getCauseNoReply())%>"  validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informNoReply value=true <%=bean.isInformNoReply() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Unconditional</th>
  <td><input class=txt name=causeUnconditional value="<%=StringEncoderDecoder.encode(bean.getCauseUnconditional())%>"  validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informUnconditional value=true <%=bean.isInformUnconditional() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Absent</th>
  <td><input class=txt name=causeAbsent value="<%=StringEncoderDecoder.encode(bean.getCauseAbsent())%>"  validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informAbsent value=true <%=bean.isInformAbsent() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Default</th>
  <td><input class=txt name=causeOther value="<%=StringEncoderDecoder.encode(bean.getCauseOther())%>"  validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informOther value=true <%=bean.isInformOther() ? "checked" : ""%>></td>
</tr>
</table>
<div class=page_subtitle>Data Source</div>
<table class=properties_list cellspacing=0  width="100%" <%rowN=0;%>>
<col width="20%">
<col width="80%">
<tr class=row<%=rowN++&1%>>
  <th>Data source type</th>
  <td><input class=txt name=dataSourceType value="<%=StringEncoderDecoder.encode(bean.getDataSourceType())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Connections</th>
  <td><input class=txt name=dataSourceConnections value="<%=StringEncoderDecoder.encode(bean.getDataSourceConnections())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB instance</th>
  <td><input class=txt name=dataSourceDbInstance value="<%=StringEncoderDecoder.encode(bean.getDataSourceDbInstance())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB user name</th>
  <td><input class=txt name=dataSourceDbUserName value="<%=StringEncoderDecoder.encode(bean.getDataSourceDbUserName())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB user password</th>
  <td><input class=txt name=dataSourceDbUserPassword value="<%=StringEncoderDecoder.encode(bean.getDataSourceDbUserPassword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=watchdog>Watchdog</label></th>
  <td><input class=check type=checkbox name=dataSourceWatchdog id=watchdog value=true <%=bean.isDataSourceWatchdog() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>JDBC driver</th>
  <td><input class=txt name=dataSourceJdbcDriver value="<%=StringEncoderDecoder.encode(bean.getDataSourceJdbcDriver())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>JDBC source</th>
  <td><input class=txt name=dataSourceJdbcSource value="<%=StringEncoderDecoder.encode(bean.getDataSourceJdbcSource())%>"></td>
</tr>
</table>
<div class=page_subtitle>MCI Profiler options</div>
<table class=properties_list cellspacing=0  width="100%" <%rowN=0;%>>
<col width="20%">
<col width="80%">
<tr class=row<%=rowN++&1%>>
  <th>location</th>
  <td><input class=txt name=mciProfLocation value="<%=StringEncoderDecoder.encode(bean.getMciProfLocation())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Add more options...</th>
  <td>&nbsp;</td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbDone",  "Done",  "Done editing");
page_menu_button(out, "mbCancel", "Cancel", "Cancel changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>