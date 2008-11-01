<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, ru.novosoft.smsc.util.Functions,
                 java.text.SimpleDateFormat"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.smsquiz.beans.Options" />
<jsp:setProperty name="bean" property="*"/>
<%
   ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
	 TITLE = getLocString("smsquiz.title");
   bean.process(request);
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0>
<col width="1%">
<col width="99%">
<% int rowN = 0;%>
<tr>
  <th align="center" colspan="2">DB</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB User</th>
  <td><input class=txt name=dbUser value="<%=StringEncoderDecoder.encode(bean.getDbUser())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB Password</th>
  <td><input class=txt name=dbPassword value="<%=StringEncoderDecoder.encode(bean.getDbPassword())%>" maxlength="5"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DB Source</th>
  <td><input class=txt name=dbSource value="<%=StringEncoderDecoder.encode(bean.getDbSource())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2">Reply statistics</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Replies directory name</th>
  <td><input class=txt name=replyDirName value="<%=StringEncoderDecoder.encode(bean.getReplyDirName())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Files collector start delay</th>
  <td><input class=txt name=fileCollectorDelay value="<%=StringEncoderDecoder.encode(bean.getFileCollectorDelay())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Files collector's period</th>
  <td><input class=txt name=fileCollectorPeriod value="<%=StringEncoderDecoder.encode(bean.getFileCollectorPeriod())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Files opened limit</th>
  <td><input class=txt name=fileOpenedLimit value="<%=StringEncoderDecoder.encode(bean.getFileOpenedLimit())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2">Distribution</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Console's host</th>
  <td><input class=txt name=consoleHost value="<%=StringEncoderDecoder.encode(bean.getConsoleHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Console's port</th>
  <td><input class=txt name=consolePort value="<%=StringEncoderDecoder.encode(bean.getConsolePort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Console's user</th>
  <td><input class=txt name=consoleUser value="<%=StringEncoderDecoder.encode(bean.getConsoleUser())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Console's password</th>
  <td><input class=txt name=consolePasssword value="<%=StringEncoderDecoder.encode(bean.getConsolePasssword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Console's success code</th>
  <td><input class=txt name=codeOk value="<%=StringEncoderDecoder.encode(bean.getCodeOk())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Console's connection timeout</th>
  <td><input class=txt name=connectTimeout value="<%=StringEncoderDecoder.encode(bean.getConnectTimeout())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Console's closer period</th>
  <td><input class=txt name=connectCloserPeriod value="<%=StringEncoderDecoder.encode(bean.getConnectCloserPeriod())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>InfoSme's success deliveru</th>
  <td><input class=txt name=infoSmeSuccDeliveryStat value="<%=StringEncoderDecoder.encode(bean.getInfoSmeSuccDeliveryStat())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>InfoSme's stats dir</th>
  <td><input class=txt name=infoSmeStatsDir value="<%=StringEncoderDecoder.encode(bean.getInfoSmeStatsDir())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>InfoSme's status checker's delay</th>
  <td><input class=txt name=statusCheckerDelay value="<%=StringEncoderDecoder.encode(bean.getStatusCheckerDelay())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>InfoSme's status checker's period</th>
  <td><input class=txt name=statusCheckerPeriod value="<%=StringEncoderDecoder.encode(bean.getStatusCheckerPeriod())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2">QuizManager</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Quizes directory</th>
  <td><input class=txt name=quizDir value="<%=StringEncoderDecoder.encode(bean.getQuizDir())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Quizes status dir</th>
  <td><input class=txt name=quizStatusDir value="<%=StringEncoderDecoder.encode(bean.getQuizStatusDir())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DirListener delay</th>
  <td><input class=txt name=dirListenerDelay value="<%=StringEncoderDecoder.encode(bean.getDirListenerDelay())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>DirListener period</th>
  <td><input class=txt name=dirListenerPeriod value="<%=StringEncoderDecoder.encode(bean.getDirListenerPeriod())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>QuizCollector delay</th>
  <td><input class=txt name=quizCollDelay value="<%=StringEncoderDecoder.encode(bean.getQuizCollDelay())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>QuizCollector period</th>
  <td><input class=txt name=quizCollPeriod value="<%=StringEncoderDecoder.encode(bean.getQuizCollPeriod())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Results dir</th>
  <td><input class=txt name=dirResults value="<%=StringEncoderDecoder.encode(bean.getDirResults())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Working dir</th>
  <td><input class=txt name=dirWork value="<%=StringEncoderDecoder.encode(bean.getDirWork())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2">JMX</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>JMX password</th>
  <td><input class=txt name=jmxPassword value="<%=StringEncoderDecoder.encode(bean.getJmxPassword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>JMX user</th>
  <td><input class=txt name=jmxUser value="<%=StringEncoderDecoder.encode(bean.getJmxUser())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2">SMPP</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMPP host</th>
  <td><input class=txt name=smppConnHost value="<%=StringEncoderDecoder.encode(bean.getSmppConnHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMPP port</th>
  <td><input class=txt name=smppConnPort value="<%=StringEncoderDecoder.encode(bean.getSmppConnPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMPP systemId</th>
  <td><input class=txt name=smppConnSystemId value="<%=StringEncoderDecoder.encode(bean.getSmppConnSystemId())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMPP password</th>
  <td><input class=txt name=smppConnPassword value="<%=StringEncoderDecoder.encode(bean.getSmppConnPassword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMPP systemType</th>
  <td><input class=txt name=smppSystemType value="<%=StringEncoderDecoder.encode(bean.getSmppSystemType())%>"></td>
</tr>

</table>

</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",  "smsquiz.done_editing");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "smsquiz..cancel_changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>