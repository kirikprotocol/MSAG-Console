<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.infosme.beans.Index,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
	ServiceIDForShowStatus = Constants.INFO_SME_ID;
	TITLE="Informer SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";
	int beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<script>
function clickAll(isChecked)
{
  opForm.all.tasksRow.disabled = isChecked;
  opForm.all.tasksCheck.disabled = isChecked;
  opForm.all.schedsRow.disabled = isChecked;
  opForm.all.schedsCheck.disabled = isChecked;
  opForm.all.tasksCheck.checked = isChecked;
  opForm.all.schedsCheck.checked = isChecked;
}
function clickTasks(isChecked)
{
  opForm.all.schedsRow.disabled = isChecked;
  opForm.all.schedsCheck.disabled = isChecked;
  opForm.all.schedsCheck.checked = isChecked;
}
</script>
<%! private String status(boolean isChanged)
{
  if (isChanged)
    return "<span style='color:red;'>changed</span>";
  else
    return "clear";
}
%>
<table class=properties_list cellspacing=0>
<col width="1%">
<col width="79%">
<col width="20%">
<tr class=row0>
  <td><input class=check type=checkbox name=apply value=all id=allCheck onClick="clickAll(this.checked); checkCheckboxes(opForm.all.mbApply);"></td>
  <th><label for=allCheck>All</label></th>
  <td><%=status(bean.isChangedAll())%></td>
</tr>
<tr class=row1 id=tasksRow>
  <td><input class=check type=checkbox name=apply value=tasks id=tasksCheck onClick="clickTasks(this.checked); checkCheckboxes(opForm.all.mbApply);"></td>
  <th><label for=tasksCheck>Tasks</label></th>
  <td><%=status(bean.isChangedTasks())%></td>
</tr>
<tr class=row0 id=schedsRow>
  <td><input class=check type=checkbox name=apply value=scheds id=schedsCheck onCLick="checkCheckboxes(opForm.all.mbApply);"></td>
  <th><label for=schedsCheck>Schedules</label></th>
  <td><%=status(bean.isChangedShedules())%></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply changes");
page_menu_button(out, "mbResetAll",  "Reset",  "Discard changes", bean.isChangedAll() || bean.isChangedTasks() || bean.isChangedShedules());
page_menu_space(out);
page_menu_end(out);
%><script>checkCheckboxes(opForm.all.mbApply);</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>