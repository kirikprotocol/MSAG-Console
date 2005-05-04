<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants, ru.novosoft.smsc.mcisme.beans.Rules,
			     ru.novosoft.smsc.jsp.SMSCJspException, ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.util.StringEncoderDecoder, ru.novosoft.smsc.mcisme.beans.MCISmeBean,
                 java.net.URLEncoder, java.util.*, ru.novosoft.smsc.util.Functions"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Rules" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Missed Calls Info SME Administration";
  MENU0_SELECTION = "MENU0_SERVICES";
  int beanResult = bean.process(request);
  switch (beanResult)
  {
    case MCISmeBean.RESULT_RULE_ADD:
      response.sendRedirect("rule.jsp?createRule=true&ruleName=");
      return;
    case MCISmeBean.RESULT_RULE_EDIT:
      response.sendRedirect("rule.jsp?createRule=false&ruleName="+
                            URLEncoder.encode(bean.getEditRule(), Functions.getLocaleEncoding()));
      return;
    default: {
      %><%@ include file="inc/menu_switch.jsp"%><%
    }
  }
int rowN=1;
List rulesList = bean.getRules();
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<script>
function editMCISmeRule(editObjectName, editButtonName)
{
  opForm.all.jbutton.name   = editButtonName;
  opForm.all.jbutton.value  = editButtonName;
  opForm.all.editRule.value = editObjectName;
  opForm.submit();
  return false;
}
function checkCheckboxesForRules() {
  return checkCheckboxes(opForm.all.mbDelete);
}
</script>
<div class=content>
<input type=hidden name=editRule value="<%=bean.getEditRule()%>">
<div class=page_subtitle>Redirect rules</div>
<table id="rules_table" cellspacing=5>
<tr><td>
  <table class=list cellspacing=1 width="100%">
  <col width="1%"  align=left><col width="5%"  align=left><col width="94%" align=left>
  <tr class=row<%=(rowN++)&1%>>
    <th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt="&nbsp;"></th>
    <th nowrap valign=top style="text-align: left">Rule name</th>
    <th nowrap valign=top style="text-align: left">Priority</th>
  </tr>
  <%for (Iterator i=rulesList.iterator(); i.hasNext();) {
    Rules.Identity id = (Rules.Identity)i.next();
    String encName = StringEncoderDecoder.encode(id.name);%>
    <tr class=row<%=(rowN++)&1%>>
      <td><input class=check type=checkbox name=informChecked
                 value="<%=encName%>" <%=bean.isRuleChecked(id.name) ? "checked":""%>
                 onClick="checkCheckboxesForRules();"></td>
      <td><%= id.priority%></td>
      <td><a href="#" title="Edit rule"
             onClick='return editMCISmeRule("<%=encName%>", "mbEdit");'><%=encName%></a></td>
    </tr><%
  }%>
  </table>
</td></tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbAdd",    "Add new",  "Add new rule");
page_menu_button(out, "mbDelete", "Delete rule(s)", "Delete selected rule(s)",
                      "return confirm('Are you sure to delete all checked rule(s)?');");
page_menu_space(out);
page_menu_end(out);
%>
<script>checkCheckboxesForRules();</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
