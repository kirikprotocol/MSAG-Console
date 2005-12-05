<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder,
				 ru.novosoft.smsc.mcisme.beans.MCISmeBean"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Rule" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE=getLocString("mcisme.title");
  MENU0_SELECTION = "MENU0_SERVICES";
  int beanResult = bean.process(request);
  switch(beanResult)
  {
	case MCISmeBean.RESULT_DONE:
		response.sendRedirect("rules.jsp");
		return;
	default: {
      %><%@ include file="inc/menu_switch.jsp"%><%
    }
  }
  int rowN=1;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%><%
  String rulePageSubtitle = getLocString("mcisme.subtitle.rule_"+(bean.isCreateRule() ? "c":"m"));
%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=createRule  value=<%=bean.isCreateRule()%>>
<input type=hidden name=oldRuleName value="<%=bean.getOldRuleName()%>">
<div class=page_subtitle><%= StringEncoderDecoder.encode(rulePageSubtitle)%></div>
<table class=properties_list cellspacing=0 cellpadding=4 width="100%" id="ruleTable">
<col width="1%" align=left>
<col width="99%" align=left>
<tr class=row<%=(rowN++)&1%>>
  <th>Rule name</th>
  <td><input class=txt name=ruleName value="<%=StringEncoderDecoder.encode(bean.getRuleName())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
  <th>Rule priority</th>
  <td><input class=txt name=priority value="<%=bean.getPriority()%>" maxlength="10" validation="id" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
  <th>Inform</th>
  <td><input class=check type=checkbox name=inform value=true <%=bean.isInform() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
  <th>Cause</th>
  <td><input class=txt name=cause value="<%=bean.getCause()%>" maxlength="10" validation="id" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN+1)&1%>>
  <th valign=top>Reg-Exp</th>
  <td valign=top><textarea name=regexp><%=StringEncoderDecoder.encode(bean.getRegexp())%></textarea></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",   "mcisme.hint.done_edit");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "mcisme.hint.cancel_changes");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>