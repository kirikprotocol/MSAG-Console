<%@ page import="ru.novosoft.smsc.jsp.smsc.region.RegionEditBean,
                 ru.novosoft.smsc.jsp.smsc.region.RegionEditBean"%>
<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.smsc.region.RegionEditBean" />
<jsp:setProperty name="bean" property="*"/>

<%
  TITLE=getLocString("infosme.title");
  MENU0_SELECTION = "MENU0_SERVICES";

  int beanResult = bean.process(request);
	switch(beanResult)
	{
		case RegionEditBean.RESULT_SAVE:
		case RegionEditBean.RESULT_CANCEL:
			response.sendRedirect("regions.jsp");
			return;
		default:
	}
  int rowN = 0;
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<script>
var global_counter = 0;
function removeSrcSubj(rowId) {
  var selectElem = document.getElementById('srcSubjSelect');
  var tbl = document.getElementById('sources_table');
  var rowElem = tbl.rows[rowId];
  var subjObj = findChildById(rowElem, 'subjSrc');
  var subjValue = subjObj.value;
  var oOption = document.createElement("OPTION");
  try {
    selectElem.add(oOption, null); // For Firefox, does not work in MSIE
  } catch (ex) {
    selectElem.add(oOption); // For MSIE
  }
  oOption.innerText = oOption.value = oOption.text = subjValue;
  selectElem.disabled = false;
  tbl.deleteRow(rowElem.rowIndex);
}

function addSourceSubj() {
  var selectElem = document.getElementById('srcSubjSelect');
  if (selectElem.options.length > 0) {
    var subjValue = selectElem.options[selectElem.selectedIndex].value;
    var tbl = document.getElementById('sources_table');
    var newRow = tbl.insertRow(tbl.rows.length);
    newRow.className = "row" + ((tbl.rows.length + 1) & 1);
    newRow.id = "srcRow_" + (global_counter++);
    newCell = document.createElement("td");
    newCell.innerHTML = '<img src="/images/subject.gif">';
    newRow.appendChild(newCell);
    newCell = document.createElement("td");
    newCell.innerHTML = subjValue + '<input id=subjSrc type=hidden name=checkedSources value="' + subjValue + '">';
    newRow.appendChild(newCell);
    newCell = document.createElement("td");
    newCell.innerHTML = '<img class=button src="/images/but_del.gif" onClick="removeSrcSubj(\'' + newRow.id + '\');">';
    newRow.appendChild(newCell);
    selectElem.options[selectElem.selectedIndex] = null;
    selectElem.focus();
    if (selectElem.options.length == 0)
      selectElem.disabled = true;
  }
}
</script>
<div class=content>
  <input type=hidden name=id id=id value=<%=bean.getId() == null ? "" : bean.getId()%>>
  <table class=properties_list>
    <col width="1%">
    <col width="49%">
    <col width="50%">
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("regions.name")%></th>
      <td><input type=txt name=name id=name value="<%=bean.getName() == null ? "" : bean.getName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
      <td>&nbsp;</td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("regions.bandwidth")%></th>
      <td><input type=txt name=sendSpeed id=sendSpeed value="<%=bean.getSendSpeed() == null ? "" : bean.getSendSpeed()%>" validation="unsigned" onkeyup="resetValidation(this)"></td>
      <td>&nbsp;</td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("regions.email")%></th>
      <td><input type=txt name=email id=email value="<%=bean.getEmail() == null ? "" : bean.getEmail()%>"></td>
      <td>&nbsp;</td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <th><%= getLocString("regions.timezone")%></th>
      <td>
        <select name="timezone" id="timezone">
        <% for (Iterator iter = bean.getTimezones().iterator(); iter.hasNext();) { String tzid = (String)iter.next();%>
          <option id="<%=tzid%>" <%=bean.getTimezone().equals(tzid) ? "SELECTED" : ""%>><%=tzid%></option>
        <% } %>
        </select>
      </td>
      <td>&nbsp;</td>
    </tr>
    <tr class=row<%=rowN++&1%>>
      <td valign=top colspan=2><%rowN = 0;%>
        <div class=page_subtitle><%=getLocString("common.titles.subjects")%></div>
        <table cellspacing=0 cellpadding=0>
          <col width="50%" align=left>
          <col width="50%" align=right>
          <col width="0%" align=left>
          <tr valign="middle">
            <td><%=getLocString("common.util.Subject")%></td>
            <td align=RIGHT><select id=srcSubjSelect name="fake_name" class="txt"><%
              for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();) {
                String name = (String) i.next();
                if (!bean.isSrcChecked(name)) {
                  String encName = StringEncoderDecoder.encode(name);
                %><option value="<%=encName%>"><%=encName%></option><%
                }
              }%></select></td>
            <td><img class=button src="/images/but_add.gif" onclick="addSourceSubj()"></td>
          </tr>
        </table>
        <table id=sources_table class=properties_list cellspacing=0 cellpadding=0>
          <col width="1%">
          <col width="100%">
          <%for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();) {
              String name = (String) i.next();
              String encName = StringEncoderDecoder.encode(name);
              String rowId = "subjRow_" + StringEncoderDecoder.encodeHEX(name);
              if (bean.isSrcChecked(name)) {%>
                <tr class=row<%=(rowN++) & 1%> id="<%=rowId%>">
                  <td><img src="/images/subject.gif"></td>
                  <td><%=encName%><input id=subjSrc type=hidden name=checkedSources value="<%=encName%>"></td>
                  <td><img class=button src="/images/but_del.gif" onClick="removeSrcSubj('<%=rowId%>');"></td>
                </tr><%
              }
            }%>
        </table>
      </td>
      <td>&nbsp;</td>
    </tr>
  </table>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbApply", "common.buttons.apply", "common.buttons.apply");
  page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancel", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>