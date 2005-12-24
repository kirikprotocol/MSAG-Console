<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.mcisme.beans.Rules,
                 ru.novosoft.smsc.util.StringEncoderDecoder, ru.novosoft.smsc.mcisme.beans.MCISmeBean,
                 java.net.URLEncoder, java.util.*, ru.novosoft.smsc.util.Functions"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Rules" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE=getLocString("mcisme.title");
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
<script type="text/javascript">
function editMCISmeRule(editObjectName, editButtonName)
{
  document.getElementById('jbutton').name   = editButtonName;
  document.getElementById('jbutton').value  = editButtonName;
  document.getElementById('editRule').value = editObjectName;
  opForm.submit();
  return false;
}
function checkCheckboxesForRules() {
  return checkCheckboxes(document.getElementById('mbDelete'));
}
</script>
<div class=content>
<input type=hidden name=editRule id=editRule value="<%= bean.getEditRule()%>">
<div class=page_subtitle><%= getLocString("mcisme.subtitle.redirect_rules")%></div>
<table id="rules_table" cellspacing=5>
<tr><td>
  <table class=list cellspacing=1 width="100%">
  <col width="1%"  align=left><col width="20%" align=left><col width="79%" align=left>
  <tr class=row<%=(rowN++)&1%>>
    <th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt="&nbsp;"></th>
    <th nowrap valign=top style="text-align: left"><%= getLocString("mcisme.label.rule_name")%></th>
    <th nowrap valign=top style="text-align: left"><%= getLocString("mcisme.label.priority")%></th>
  </tr>
  <%for (Iterator i=rulesList.iterator(); i.hasNext();) {
    Rules.Identity id = (Rules.Identity)i.next();
    String encName = StringEncoderDecoder.encode(id.name);%>
    <tr class=row<%=(rowN++)&1%>>
      <td><input class=check type=checkbox name=checked
                 value="<%=encName%>" <%= (bean.isChecked(id.name) ? "checked":"")%>
                 onClick="checkCheckboxesForRules();"></td>
      <td><a href="#" title="<%= getLocString("mcisme.hint.rule_edit")%>"
             onClick='return editMCISmeRule("<%=encName%>", "mbEdit");'><%=encName%></a></td>
      <td><%= id.priority%></td>
    </tr><%
  }%>
  </table>
</td></tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",    "mcisme.button.add_new",   "mcisme.hint.add_new_rule");
page_menu_confirm_button(session, out, "mbDelete", "mcisme.button.del_rules", "mcisme.hint.del_rules", getLocString("mcisme.confirm.del_rules"));
page_menu_space(out);
page_menu_end(out);
%>
<script>checkCheckboxesForRules();</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
