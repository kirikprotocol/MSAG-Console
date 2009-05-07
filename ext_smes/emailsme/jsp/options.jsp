<%@ page import="ru.novosoft.smsc.emailsme.beans.Options,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.emailsme.beans.Options" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Email SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";

  int rowN = 0;
  int beanResult = bean.process(request);
%><%@ include file="switch_menu.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="menu.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0>
<col width="10%">

<tr><td colspan=2><div class=page_subtitle>Store</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>directory</th>
  <td><input class=txt name=store_dir value="<%=StringEncoderDecoder.encode(bean.getStore_dir())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>queue directory</th>
  <td><input class=txt name=store_queueDir value="<%=StringEncoderDecoder.encode(bean.getStore_queueDir())%>"></td>
</tr>


<%rowN=0;%>
<tr><td colspan=2><div class=page_subtitle>SMPP</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>host</th>
  <td><input class=txt name=smpp_host value="<%=StringEncoderDecoder.encode(bean.getSmpp_host())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>port</th>
  <td><input class=txt name=smpp_port value="<%=StringEncoderDecoder.encode(bean.getSmpp_port())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>timeout</th>
  <td><input class=txt name=smpp_timeout value="<%=StringEncoderDecoder.encode(bean.getSmpp_timeout())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>systemId</th>
  <td><input class=txt name=smpp_systemId value="<%=StringEncoderDecoder.encode(bean.getSmpp_systemId())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>password</th>
  <td><input class=txt name=smpp_password value="<%=StringEncoderDecoder.encode(bean.getSmpp_password())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>sourceAddress</th>
  <td><input class=txt name=smpp_sourceAddress value="<%=StringEncoderDecoder.encode(bean.getSmpp_sourceAddress())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>serviceType</th>
  <td><input class=txt name=smpp_serviceType value="<%=StringEncoderDecoder.encode(bean.getSmpp_serviceType())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>protocolId</th>
  <td><input class=txt name=smpp_protocolId value="<%=StringEncoderDecoder.encode(bean.getSmpp_protocolId())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>retryTime</th>
  <td><input class=txt name=smpp_retryTime value="<%=StringEncoderDecoder.encode(bean.getSmpp_retryTime())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>partitionSms</th>
  <td align="LEFT"><input type=checkbox name=smpp_partitionSms <%=bean.isSmpp_partitionSms() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>partsSendSpeedPerHour</th>
  <td><input class=txt name=smpp_partsSendSpeedPerHour value="<%=StringEncoderDecoder.encode(bean.getSmpp_partsSendSpeedPerHour())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>maxUdhParts</th>
  <td><input class=txt name=smpp_maxUdhParts value="<%=StringEncoderDecoder.encode(bean.getSmpp_maxUdhParts())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>concatTimeout</th>
  <td><input class=txt name=smpp_concatTimeout value="<%=StringEncoderDecoder.encode(bean.getSmpp_concatTimeout())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>pause after disconnect</th>
  <td align="LEFT"><input type=checkbox name=smpp_pauseAfterDisconnect <%=bean.isSmpp_pauseAfterDisconnect() ? "checked" : ""%>></td>
</tr>

<%rowN=0;%>
<tr><td colspan=2>&nbsp;</td></tr>
<tr><td colspan=2><div class=page_subtitle>Listener</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>host</th>
  <td><input class=txt name=listener_host value="<%=StringEncoderDecoder.encode(bean.getListener_host())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>port</th>
  <td><input class=txt name=listener_port value="<%=StringEncoderDecoder.encode(bean.getListener_port())%>"></td>
</tr>

<%rowN=0;%>
<tr><td colspan=2>&nbsp;</td></tr>
<tr><td colspan=2><div class=page_subtitle>Admin</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>host</th>
  <td><input class=txt name=admin_host value="<%=StringEncoderDecoder.encode(bean.getAdmin_host())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>port</th>
  <td><input class=txt name=admin_port value="<%=StringEncoderDecoder.encode(bean.getAdmin_port())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>allow GSM to email without profile</th>
  <td align="LEFT"><input type=checkbox name=admin_allow_gsm_2_email_without_profile <%=bean.isAdmin_allow_gsm_2_email_without_profile() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>allow email to GSM without profile</th>
  <td align="LEFT"><input type=checkbox name=admin_allow_email_2_gsm_without_profile <%=bean.isAdmin_allow_email_2_gsm_without_profile() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>default limit</th>
  <td><input class=txt name=admin_default_limit value="<%=StringEncoderDecoder.encode(bean.getAdmin_default_limit())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>regions config</th>
  <td><input class=txt name=admin_regionsconfig value="<%=StringEncoderDecoder.encode(bean.getAdmin_regionsconfig())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>routes config</th>
  <td><input class=txt name=admin_routesconfig value="<%=StringEncoderDecoder.encode(bean.getAdmin_routesconfig())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>helpdesk address</th>
  <td><input class=txt name=admin_helpdeskAddress value="<%=StringEncoderDecoder.encode(bean.getAdmin_helpdeskAddress())%>"></td>
</tr>


<%rowN=0;%>

<%rowN=0;%>
<tr><td colspan=2>&nbsp;</td></tr>
<tr><td colspan=2><div class=page_subtitle>SMTP</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>host</th>
  <td><input class=txt name=smtp_host value="<%=StringEncoderDecoder.encode(bean.getSmtp_host())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>host</th>
  <td><input class=txt name=smtp_port value="<%=StringEncoderDecoder.encode(bean.getSmtp_port())%>"></td>
</tr>

<tr class=row<%=rowN++&1%>>
<tr><td colspan=2>&nbsp;</td></tr>
<tr><td colspan=2><div class=page_subtitle>Mail</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>domain</th>
  <td><input class=txt name=mail_domain value="<%=StringEncoderDecoder.encode(bean.getMail_domain())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>stripper</th>
  <td><input class=txt name=mail_stripper value="<%=StringEncoderDecoder.encode(bean.getMail_stripper())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>format</th>
  <td><input class=txt name=mail_format value="<%=StringEncoderDecoder.encode(bean.getMail_format())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>user name transform regexp</th>
  <td><input class=txt name=mail_user_name_transform_regexp value="<%=StringEncoderDecoder.encode(bean.getMail_user_name_transform_regexp())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>user name transform result</th>
  <td><input class=txt name=mail_user_name_transform_result value="<%=StringEncoderDecoder.encode(bean.getMail_user_name_transform_result())%>"></td>
</tr>

<%rowN=0;%>
<tr><td colspan=2><div class=page_subtitle>Statistic</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>enabled</th>
  <td align="LEFT"><input type=checkbox name=stat_enabled <%=bean.isStat_enabled() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>store location</th>
  <td><input class=txt name=stat_storeLocation value="<%=StringEncoderDecoder.encode(bean.getStat_storeLocation())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>flush period (sec)</th>
  <td><input class=txt name=stat_flushPeriodInSec value="<%=StringEncoderDecoder.encode(bean.getStat_flushPeriodInSec())%>"></td>
</tr>

<%rowN=0;%>
<tr><td colspan=2>&nbsp;</td></tr>
<tr><td colspan=2><div class=page_subtitle>Defaults</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>annotation size</th>
  <td><input class=txt name=default_annotation_size value="<%=StringEncoderDecoder.encode(bean.getDefault_annotation_size())%>"></td>
</tr>

<%rowN=0;%>
<tr><td colspan=2>&nbsp;</td></tr>
<tr><td colspan=2><div class=page_subtitle>Answers</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>Send success noticies</th>
  <td align="LEFT"><input type=checkbox name=sendSuccessAnswer <%=bean.isSendSuccessAnswer() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>alias</th>
  <td><input class=txt name=answers_alias value="<%=StringEncoderDecoder.encode(bean.getAnswers_alias())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>alias failed</th>
  <td><input class=txt name=answers_aliasfailed value="<%=StringEncoderDecoder.encode(bean.getAnswers_aliasfailed())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>alias busy</th>
  <td><input class=txt name=answers_aliasbusy value="<%=StringEncoderDecoder.encode(bean.getAnswers_aliasbusy())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>no alias</th>
  <td><input class=txt name=answers_noalias value="<%=StringEncoderDecoder.encode(bean.getAnswers_noalias())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>forward</th>
  <td><input class=txt name=answers_forward value="<%=StringEncoderDecoder.encode(bean.getAnswers_forward())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>forward failed</th>
  <td><input class=txt name=answers_forwardfailed value="<%=StringEncoderDecoder.encode(bean.getAnswers_forwardfailed())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>forward off</th>
  <td><input class=txt name=answers_forwardoff value="<%=StringEncoderDecoder.encode(bean.getAnswers_forwardoff())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>real name</th>
  <td><input class=txt name=answers_realname value="<%=StringEncoderDecoder.encode(bean.getAnswers_realname())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>number on</th>
  <td><input class=txt name=answers_numberon value="<%=StringEncoderDecoder.encode(bean.getAnswers_numberon())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>number off</th>
  <td><input class=txt name=answers_numberoff value="<%=StringEncoderDecoder.encode(bean.getAnswers_numberoff())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>number failed</th>
  <td><input class=txt name=answers_numberfailed value="<%=StringEncoderDecoder.encode(bean.getAnswers_numberfailed())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>system error</th>
  <td><input class=txt name=answers_systemerror value="<%=StringEncoderDecoder.encode(bean.getAnswers_systemerror())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>unknown command</th>
  <td><input class=txt name=answers_unknowncommand value="<%=StringEncoderDecoder.encode(bean.getAnswers_unknowncommand())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>message send</th>
  <td><input class=txt name=answers_messagesent value="<%=StringEncoderDecoder.encode(bean.getAnswers_messagesent())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>message failed limit</th>
  <td><input class=txt name=answers_messagefailedlimit value="<%=StringEncoderDecoder.encode(bean.getAnswers_messagefailedlimit())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>message no profile</th>
  <td><input class=txt name=answers_messagefailednoprofile value="<%=StringEncoderDecoder.encode(bean.getAnswers_messagefailednoprofile())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>message failed send mail</th>
  <td><input class=txt name=answers_messagefailedsendmail value="<%=StringEncoderDecoder.encode(bean.getAnswers_messagefailedsendmail())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>message failed system</th>
  <td><input class=txt name=answers_messagefailedsystem value="<%=StringEncoderDecoder.encode(bean.getAnswers_messagefailedsystem())%>"></td>
</tr>

</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "Done",   "Done editing");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
