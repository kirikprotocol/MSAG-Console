<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.mcisme.beans.Offsets,
                 ru.novosoft.smsc.util.StringEncoderDecoder, ru.novosoft.smsc.mcisme.beans.MCISmeBean,
                 java.net.URLEncoder, java.util.*, ru.novosoft.smsc.util.Functions"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Offsets" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE=getLocString("mcisme.title");
  MENU0_SELECTION = "MENU0_SERVICES";
  int beanResult = bean.process(request);
  switch (beanResult)
  {
    case MCISmeBean.RESULT_OFFSET_ADD:
      response.sendRedirect("offset.jsp?createOffset=true&offsetName=");
      return;
    case MCISmeBean.RESULT_OFFSET_EDIT:
      response.sendRedirect("offset.jsp?createOffset=false&offsetName="+
                            URLEncoder.encode(bean.getEditOffset(), Functions.getLocaleEncoding()));
      return;
    default: {
      %><%@ include file="inc/menu_switch.jsp"%><%
    }
  }
int rowN=1;
List offsetsList = bean.getOffsets();
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<script type="text/javascript">
function editMCISmeOffset(editObjectName, editButtonName)
{
  document.getElementById('jbutton').name   = editButtonName;
  document.getElementById('jbutton').value  = editButtonName;
  document.getElementById('editOffset').value = editObjectName;
  opForm.submit();
  return false;
}
function checkCheckboxesForOffsets() {
  return checkCheckboxes(document.getElementById('mbDelete'));
}
</script>
<div class=content>
<input type=hidden name=editOffset id=editOffset value="<%= bean.getEditOffset()%>">
<div class=page_subtitle><%= getLocString("mcisme.subtitle.tz_rules")%></div>
<table id="offsets_table" cellspacing=5>
<tr><td>
  <table class=list cellspacing=1 width="100%">
  <col width="1%"  align=left><col width="20%" align=left><col width="79%" align=left>
  <tr class=row<%=(rowN++)&1%>>
    <th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt="&nbsp;"></th>
    <th nowrap valign=top style="text-align: left"><%= getLocString("mcisme.label.tz_o_name")%></th>
    <th nowrap valign=top style="text-align: left"><%= getLocString("mcisme.label.tzo_value")%></th>
  </tr>
  <%for (Iterator i=offsetsList.iterator(); i.hasNext();) {
    Offsets.Identity id = (Offsets.Identity)i.next();
    String encName = StringEncoderDecoder.encode(id.name);%>
    <tr class=row<%=(rowN++)&1%>>
      <td><input class=check type=checkbox name=checked
                 value="<%=encName%>" <%= (bean.isChecked(id.name) ? "checked":"")%>
                 onClick="checkCheckboxesForOffsets();"></td>
      <td><a href="#" title="<%= getLocString("mcisme.hint.tz_edit")%>"
             onClick='return editMCISmeOffset("<%=encName%>", "mbEdit");'><%=encName%></a></td>
      <td><%= ((id.offset > 0) ? "+":"")+Integer.toString(id.offset)%></td>
    </tr><%
  }%>
  </table>
</td></tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",    "mcisme.button.add_new",  "mcisme.hint.add_new_tz");
page_menu_confirm_button(session, out, "mbDelete", "mcisme.button.del_tz", "mcisme.hint.del_tz", getLocString("mcisme.confirm.tz"));
page_menu_space(out);
page_menu_end(out);
%>
<script>checkCheckboxesForOffsets();</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
