<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.Constants,
					  ru.novosoft.smsc.mcisme.beans.Circuits,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  java.util.Set, java.util.Iterator,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  java.util.Collection, ru.novosoft.smsc.mcisme.beans.MCISmeBean"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Circuits" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE=getLocString("mcisme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
    int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<script>
function clickAddCircuits()
{
  var mscElem = opForm.all.circuits_new_msc;
  var hsnElem = opForm.all.circuits_new_hsn;
  var spnElem = opForm.all.circuits_new_spn;
  var tsmElem = opForm.all.circuits_new_tsm;
  if (trim(mscElem.value) == '' || trim(hsnElem.value) == '' ||
      trim(spnElem.value) == '' || trim(hsnElem.value) == '') {
      alert('<%= getLocString("mcisme.alert.circuits_1")%>.\n<%= getLocString("mcisme.alert.circuits_2")%>');
      return false;
  }

  var newRow  = circuits_table_id.insertRow(circuits_table_id.rows.length-1);
  newRow.className = "row" + (circuits_table_id.rows.length & 1);
  newRow.id = "circuits_row_" + mscElem.value;
  var newCell = document.createElement("td");
  newCell.innerHTML = "<input class=txt  name=\"<%=Circuits.CIRCUITS_SECTION_NAME%>." + mscElem.value + ".msc\" value=\"" + mscElem.value + "\">";
  newRow.appendChild(newCell);
  newCell = document.createElement("td");
  newCell.innerHTML = "<input class=txtW name=\"<%=Circuits.CIRCUITS_SECTION_NAME%>." + mscElem.value + ".hsn\" value=\"" + hsnElem.value + "\">";
  newRow.appendChild(newCell);
  newCell = document.createElement("td");
  newCell.innerHTML = "<input class=txtW name=\"<%=Circuits.CIRCUITS_SECTION_NAME%>." + mscElem.value + ".spn\" value=\"" + spnElem.value + "\">";
  newRow.appendChild(newCell);
  newCell = document.createElement("td");
  newCell.innerHTML = "<input class=txtW name=\"<%=Circuits.CIRCUITS_SECTION_NAME%>." + mscElem.value + ".tsm\" value=\"" + tsmElem.value + "\">";
  newRow.appendChild(newCell);
  newCell = document.createElement("td");
  newCell.innerHTML = "<img src=\"/images/but_del.gif\" class=button jbuttonName=\"mbRemove\" jbuttonValue=\"common.buttons.remove\" title=\"mcisme.hint.remove_circuits\" jbuttonOnclick=\"return clickRemoveCircuits('" + newRow.id + "');\">";
  newRow.appendChild(newCell);

  mscElem.value = "";	hsnElem.value = "";
  spnElem.value = "";	tsmElem.value = "";

  return false;
}
function clickRemoveCircuits(id_to_remove)
{
	var rowElem = circuits_table_id.rows(id_to_remove);
	circuits_table_id.deleteRow(rowElem.rowIndex);

	return false;
}
</script>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.circuits_settings")%></div>
<table class=list cellspacing=0 id=circuits_table_id>
<col width=30%>
<col width=15%>
<col width=15%>
<col width=29%>
<col width=1%>
<tr>
  <th>MSC ID</th>
  <th>HSN</th>
  <th>SPN</th>
  <th colspan=2>TSM (hex)</th>
</tr>
<%
	int rowN = 0;
	Collection circuitsSectionNames = bean.getCircuitsSectionNames();
	for (Iterator i = circuitsSectionNames.iterator(); i.hasNext();) {
		String msc = (String) i.next();
    String mscSection = Circuits.CIRCUITS_SECTION_NAME + "." + msc;
		int    hsn = bean.getInt   (mscSection+".hsn");
    int    spn = bean.getInt   (mscSection+".spn");
    String tsm = bean.getString(mscSection+".tsm");
		%><tr class=row<%=(rowN++)&1%> id=circuits_row_<%=StringEncoderDecoder.encode(msc)%>>
			<td><input class=txt  name="<%=mscSection%>.msc" value="<%=msc%>"></td>
			<td><input class=txtW name="<%=mscSection%>.hsn" value="<%=hsn%>"></td>
      <td><input class=txtW name="<%=mscSection%>.spn" value="<%=spn%>"></td>
      <td><input class=txtW name="<%=mscSection%>.tsm" value="<%=tsm%>"></td>
			<td><%button(out, "but_del.gif", "mbDel", "common.buttons.remove", "mcisme.hint.remove_circuits", "return clickRemoveCircuits('circuits_row_" + StringEncoderDecoder.encode(msc) + "');");%></td>
		</tr><%
	}
%>
<tr id=circuits_new class=row<%=(rowN++)&1%>>
	<td><input class=txt  id="circuits_new_msc" name="circuits_new_msc"></td>
	<td><input class=txtW id="circuits_new_hsn" name="circuits_new_hsn"></td>
  <td><input class=txtW id="circuits_new_spn" name="circuits_new_spn"></td>
  <td><input class=txtW id="circuits_new_tsm" name="circuits_new_tsm"></td>
	<td><%button(out, "but_add.gif", "mbAdd", "common.buttons.add", "mcisme.hint.add_circuits", "return clickAddCircuits();");%></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",   "mcisme.hint.done_edit");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "mcisme.hint.cancel_changes");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>