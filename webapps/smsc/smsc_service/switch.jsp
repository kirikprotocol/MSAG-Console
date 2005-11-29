<%@
 include file="/WEB-INF/inc/code_header.jsp"%><%@
 page import="ru.novosoft.smsc.jsp.smsc.smsc_service.Index,
				  ru.novosoft.smsc.admin.service.ServiceInfo,
				  ru.novosoft.smsc.admin.Constants,
				  java.io.IOException,
				  ru.novosoft.smsc.util.StringEncoderDecoder,
				  java.util.Iterator,
				  ru.novosoft.smsc.jsp.SMSCErrors,
				  ru.novosoft.smsc.jsp.SMSCJspException,
              ru.novosoft.smsc.jsp.smsc.smsc_service.Switch,
              java.util.Collection,
              java.util.Arrays,
              ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
              ru.novosoft.smsc.jsp.util.tables.DataItem,
              ru.novosoft.smsc.admin.smsc_service.Smsc"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.smsc_service.Switch"
/><jsp:setProperty name="bean" property="*"/><%
  FORM_METHOD = "POST";
	TITLE = getLocString("smsc.title");
    isServiceStatusNeeded = true;
	MENU0_SELECTION = "MENU0_SMSC_SWITCH";
	switch (bean.process(request))
	{
		case Index.RESULT_OK:
			break;
		case Index.RESULT_ERROR:
			break;
		default:
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
	}
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%><%
  if ((errorMessages.size() == 0)||(!((SMSCJspException) errorMessages.get(0)).getMessage().equals(SMSCErrors.error.smsc.contextInitFailed))) {
  page_menu_begin(out);
  page_menu_button(session, out, "mbOnline", "common.buttons.online", "smsc.start");
  page_menu_button(session, out, "mbOffline", "common.buttons.offline", "smsc.stop");
  page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "smsc.activateHint");
  page_menu_button(session, out, "mbApplyConfig", "common.buttons.applyConfig", "smsc.masterHint");
  page_menu_button(session, out, "mbSwitchOver", "common.buttons.switchOver", "smsc.masterHint");
  page_menu_space(out);
  page_menu_end(out);%>
<div class=content>
<table class=list cellspacing=1 cellpadding=1 id=SMSC_LIST_TABLE>
<col width="5%" align=left>
<col width="35%" align=left>
<col width="30%" align=left>
<col width="30%" align=left>
<thead>
<tr>
	<th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th><%=getLocString("common.sortmodes.name")%></th>
	<th><%=getLocString("common.sortmodes.node")%></th>
	<th><%=getLocString("common.sortmodes.status")%></th>
</tr>
</thead>
<tbody>
<%
  Map smscenters = bean.getAppContext().getSmscList().getSmsCenters();
  int n = 0;
  String[] nodes = bean.getAppContext().getSmscList().getNodes();
  for (Iterator i = smscenters.keySet().iterator(); i.hasNext();)
  {
      Smsc smsc = (Smsc) smscenters.get(i.next());
%>
<tr class=row<%=n%>>
  <td><input class=check type=radio name=checkedSmsc value="<%=nodes[n]%>"></td>
  <td><%=smsc.getInfo().getId()%></td>
  <td><%=nodes[n]%></td>
  <td><%=bean.getAppContext().getHostsManager().getService("SMSC").getOnlineStatus(nodes[n])%></td>
</tr>
<%n++;}
%>
</tbody>
</table>
</div><%
  page_menu_begin(out);
  page_menu_button(session, out, "mbOnline", "common.buttons.online", "smsc.start");
  page_menu_button(session, out, "mbOffline", "common.buttons.offline", "smsc.stop");
  page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "smsc.activateHint");
  page_menu_button(session, out, "mbApplyConfig", "common.buttons.applyConfig", "smsc.masterHint");
  page_menu_button(session, out, "mbSwitchOver", "common.buttons.switchOver", "smsc.masterHint");
  page_menu_space(out);
  page_menu_end(out);}
%><%@
 include file="/WEB-INF/inc/html_3_footer.jsp"%><%@
 include file="/WEB-INF/inc/code_footer.jsp"%>