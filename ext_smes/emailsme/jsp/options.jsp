<%@ page import="ru.novosoft.smsc.emailsme.beans.Options,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.emailsme.beans.Options" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Email SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";

  int rowN = 0;
  int beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal);
%><%@ include file="switch_menu.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="menu.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list cellspacing=0>
<col width="10%">
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
<tr><td colspan=2><div class=page_subtitle>Mail</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>domain</th>
  <td><input class=txt name=mail_domain value="<%=StringEncoderDecoder.encode(bean.getMail_domain())%>"></td>
</tr>

<%rowN=0;%>
<tr><td colspan=2>&nbsp;</td></tr>
<tr><td colspan=2><div class=page_subtitle>SMTP</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>host</th>
  <td><input class=txt name=smtp_host value="<%=StringEncoderDecoder.encode(bean.getSmtp_host())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>port</th>
  <td><input class=txt name=smtp_port value="<%=StringEncoderDecoder.encode(bean.getSmtp_port())%>"></td>
</tr>

<%rowN=0;%>
<tr><td colspan=2>&nbsp;</td></tr>
<tr><td colspan=2><div class=page_subtitle>Defaults</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>daily limit</th>
  <td><input class=txt name=defaults_dailyLimt value="<%=StringEncoderDecoder.encode(bean.getDefaults_dailyLimt())%>"></td>
</tr>

<%rowN=0;%>
<tr><td colspan=2>&nbsp;</td></tr>
<tr><td colspan=2><div class=page_subtitle>DataSource</div></td></tr>
<tr class=row<%=rowN++&1%>>
  <th>type</th>
  <td><input class=txt name=dataSource_type value="<%=StringEncoderDecoder.encode(bean.getDataSource_type())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>connections</th>
  <td><input class=txt name=dataSource_connections value="<%=StringEncoderDecoder.encode(bean.getDataSource_connections())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>dbInstance</th>
  <td><input class=txt name=dataSource_dbInstance value="<%=StringEncoderDecoder.encode(bean.getDataSource_dbInstance())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>dbUserName</th>
  <td><input class=txt name=dataSource_dbUserName value="<%=StringEncoderDecoder.encode(bean.getDataSource_dbUserName())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>dbUserPassword</th>
  <td><input class=txt name=dataSource_dbUserPassword value="<%=StringEncoderDecoder.encode(bean.getDataSource_dbUserPassword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>JDBC source</th>
  <td><input class=txt name=dataSource_jdbc_source value="<%=StringEncoderDecoder.encode(bean.getDataSource_jdbc_source())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>JDBC driver</th>
  <td><input class=txt name=dataSource_jdbc_driver value="<%=StringEncoderDecoder.encode(bean.getDataSource_jdbc_driver())%>"></td>
</tr>

</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbDone",   "Done",   "Done editing");
page_menu_button(out, "mbCancel", "Cancel", "Cancel changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
