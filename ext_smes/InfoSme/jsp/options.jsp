<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.infosme.beans.Options,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.infosme.beans.Index,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.*"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Options" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE="Informer SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0>
<col width="1%">
<col width="99%">
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
  <th>Max messages per second</th>
  <td><input class=txt name=maxMessagesPerSecond value="<%=StringEncoderDecoder.encode(bean.getMaxMessagesPerSecond())%>"></td>
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
  <th>Tasks task tables prefix</th>
  <td><input class=txt name=tasksTaskTablesPrefix value="<%=StringEncoderDecoder.encode(bean.getTasksTaskTablesPrefix())%>" maxlength="10"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks switch timeout</th>
  <td><input class=txt name=tasksSwitchTimeout value="<%=StringEncoderDecoder.encode(bean.getTasksSwitchTimeout())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks thread pool max</th>
  <td><input class=txt name=tasksThreadPoolMax value="<%=StringEncoderDecoder.encode(bean.getTasksThreadPoolMax())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Tasks thread pool init</th>
  <td><input class=txt name=tasksThreadPoolInit value="<%=StringEncoderDecoder.encode(bean.getTasksThreadPoolInit())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Events thread pool max</th>
  <td><input class=txt name=eventsThreadPoolMax value="<%=StringEncoderDecoder.encode(bean.getEventsThreadPoolMax())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Events thread pool init</th>
  <td><input class=txt name=eventsThreadPoolInit value="<%=StringEncoderDecoder.encode(bean.getEventsThreadPoolInit())%>"></td>
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
<div class=page_subtitle>System data source</div>
<table class=properties_list <%rowN=0;%>>
<col width="10%">
<tr class=row<%=rowN++&1%>>
  <th>Data source type</th>
  <td><input class=txt name=systemDataSourceType value="<%=StringEncoderDecoder.encode(bean.getSystemDataSourceType())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Connections</th>
  <td><input class=txt name=systemDataSourceConnections value="<%=StringEncoderDecoder.encode(bean.getSystemDataSourceConnections())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB instance</th>
  <td><input class=txt name=systemDataSourceDbInstance value="<%=StringEncoderDecoder.encode(bean.getSystemDataSourceDbInstance())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB user name</th>
  <td><input class=txt name=systemDataSourceDbUserName value="<%=StringEncoderDecoder.encode(bean.getSystemDataSourceDbUserName())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB user password</th>
  <td><input class=txt name=systemDataSourceDbUserPassword value="<%=StringEncoderDecoder.encode(bean.getSystemDataSourceDbUserPassword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><label for=watchdog>Watchdog</label></th>
  <td><input class=check type=checkbox name=systemDataSourceWatchdog id=watchdog value=true <%=bean.isSystemDataSourceWatchdog() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>JDBC driver</th>
  <td><input class=txt name=systemDataSourceJdbcDriver value="<%=StringEncoderDecoder.encode(bean.getSystemDataSourceJdbcDriver())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>JDBC source</th>
  <td><input class=txt name=systemDataSourceJdbcSource value="<%=StringEncoderDecoder.encode(bean.getSystemDataSourceJdbcSource())%>"></td>
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