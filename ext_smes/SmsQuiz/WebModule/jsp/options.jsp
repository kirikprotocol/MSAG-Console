<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*, ru.novosoft.smsc.util.Functions,
                 java.text.SimpleDateFormat"%>
<%@ page import="mobi.eyeline.smsquiz.beans.Options"%>
<jsp:useBean id="bean" scope="page" class="mobi.eyeline.smsquiz.beans.Options" />
<jsp:setProperty name="bean" property="*"/>
<%
  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
  TITLE = getLocString("smsquiz.title")+": "+getLocString("smsquiz.label.options");
  int beanResult = bean.process(request);
  if(beanResult == Options.RESULT_SAVED) {
    session.setAttribute("message_to_restart","restart");
    beanResult = Options.RESULT_DONE;
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
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
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.db")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.dbuser")%></th>
  <td><input class=txt name=dbUser value="<%=StringEncoderDecoder.encode(bean.getDbUser())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.dbpassword")%></th>
  <td><input class=txt name=dbPassword value="<%=StringEncoderDecoder.encode(bean.getDbPassword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.dbsource")%></th>
  <td><input class=txt validation="nonEmpty" name=dbSource value="<%=StringEncoderDecoder.encode(bean.getDbSource())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.reply")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.replydir")%></th>
  <td><input class=txt validation="nonEmpty" name=replyDirName value="<%=StringEncoderDecoder.encode(bean.getReplyDirName())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.distribution")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.console.host")%></th>
  <td><input class=txt validation="nonEmpty" name=consoleHost value="<%=StringEncoderDecoder.encode(bean.getConsoleHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.console.port")%></th>
  <td><input class=txt validation="port" name=consolePort value="<%=StringEncoderDecoder.encode(bean.getConsolePort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.console.user")%></th>
  <td><input class=txt name=consoleUser value="<%=StringEncoderDecoder.encode(bean.getConsoleUser())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.console.password")%></th>
  <td><input class=txt name=consolePasssword value="<%=StringEncoderDecoder.encode(bean.getConsolePasssword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.infosme.stats_dir")%></th>
  <td><input class=txt validation="nonEmpty" name=infoSmeStatsDir value="<%=StringEncoderDecoder.encode(bean.getInfoSmeStatsDir())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.quizmanager")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.quiz.dir")%></th>
  <td><input class=txt validation="nonEmpty" name=quizDir value="<%=StringEncoderDecoder.encode(bean.getQuizDir())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.quiz.listener.period")%></th>
  <td><input class=txt validation="positive" name=dirListenerPeriod value="<%=StringEncoderDecoder.encode(bean.getDirListenerPeriod())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.quiz.res_dir")%></th>
  <td><input class=txt validation="nonEmpty" name=dirResults value="<%=StringEncoderDecoder.encode(bean.getDirResults())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.quiz.work_dir")%></th>
  <td><input class=txt validation="nonEmpty" name=dirWork value="<%=StringEncoderDecoder.encode(bean.getDirWork())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.quiz.arch_dir")%></th>
  <td><input class=txt validation="nonEmpty" name=archiveDir value="<%=StringEncoderDecoder.encode(bean.getArchiveDir())%>"></td>
</tr>
<tr>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.smpp")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.smpp.host")%></th>
  <td><input class=txt validation="nonEmpty" name=smppConnHost value="<%=StringEncoderDecoder.encode(bean.getSmppConnHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.smpp.port")%></th>
  <td><input class=txt validation="port" name=smppConnPort value="<%=StringEncoderDecoder.encode(bean.getSmppConnPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.smpp.system.id")%></th>
  <td><input class=txt name=smppConnSystemId value="<%=StringEncoderDecoder.encode(bean.getSmppConnSystemId())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.smpp.password")%></th>
  <td><input class=txt name=smppConnPassword value="<%=StringEncoderDecoder.encode(bean.getSmppConnPassword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.smpp.system.type")%></th>
  <td><input class=txt name=smppSystemType value="<%=StringEncoderDecoder.encode(bean.getSmppSystemType())%>"></td>
</tr>

<tr>
  <th align="center" colspan="2"><div class=page_subtitle><%=getLocString("smsquiz.label.commandserver")%></div></th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.commandserver.port")%></th>
  <td><input class=txt validation="port" name=commandPort value="<%=StringEncoderDecoder.encode(bean.getCommandPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th><%=getLocString("smsquiz.label.commandserver.host")%></th>
  <td><input class=txt validation="nonEmpty" name=commandHost value="<%=StringEncoderDecoder.encode(bean.getCommandHost())%>"></td>
</tr>

</table>

</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",  "smsquiz.done_editing");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "smsquiz.cancel_changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>