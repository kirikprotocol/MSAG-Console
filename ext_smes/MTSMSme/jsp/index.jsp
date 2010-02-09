<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.mtsmsme.beans.Index,
                 java.util.*, ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.util.Functions,
                 ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper"%>
<%@ page import="java.io.IOException"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mtsmsme.beans.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
  ServiceIDForShowStatus = Functions.getServiceId(request.getServletPath());
  TITLE=getLocString("mtsmsme.title");
  MENU0_SELECTION = "MENU0_SERVICES";
  int beanResult = bean.process(request);
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%><%
page_menu_begin(out);
page_menu_space(out);
page_menu_button(session, out, "mbStart", "common.buttons.start", "mtsmsme.hint.start");
page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "mtsmsme.hint.stop");
page_menu_end(out);%>
<script  type="text/javascript" language="JavaScript">
function checkStartStop()
{
    var status = document.getElementById('RUNNING_STATUSERVICE_<%=ServiceIDForShowStatus%>').innerText;
    document.getElementById('mbStart').disabled = (status != "<%= getLocString("common.statuses.offline") %>");
    document.getElementById('mbStop').disabled  = (status == "<%= getLocString("common.statuses.offline") %>");
    window.setTimeout(checkStartStop, 5000);
}

function showhide_section(sectionId) {
  var headerElem = document.getElementById("sectionHeader_" + sectionId);

  var valueElem = document.getElementById("sectionValue_" + sectionId);
  if (valueElem.style.display != "none")
  {
    headerElem.src = "/images/list_closed.gif";
    valueElem.style.display = "none";
  }
  else
  {
    headerElem.src = "/images/list_opened.gif";
    valueElem.style.display = "";
  }

}
checkStartStop();
</script>
<div class=content>
<input type=hidden name=initialized value=true>
<div class=page_subtitle><%= getLocString("mtsmsme.subtitle.smsc_connection")%></div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%">
<col width="80%">
<% int rowN = 0;%>
<tr class=row<%=rowN++&1%>>
  <th>SMSC host</th>
  <td><input class=txt name=smscHost value="<%=StringEncoderDecoder.encode(bean.getSmscHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC port</th>
  <td><input class=txt name=smscPort value="<%=StringEncoderDecoder.encode(bean.getSmscPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC SID</th>
  <td><input class=txt name=smscSid value="<%=StringEncoderDecoder.encode(bean.getSmscSid())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC connection timeout</th>
  <td><input class=txt name=smscTimeout value="<%=StringEncoderDecoder.encode(bean.getSmscTimeout())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC connection password</th>
  <td><input class=txt name=smscPassword value="<%=StringEncoderDecoder.encode(bean.getSmscPassword())%>"></td>
</tr>
</table><br/>

<div class=page_subtitle><%= getLocString("mtsmsme.subtitle.sccp_connection")%></div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%">
<col width="80%">
<% rowN = 0;%>
<tr class=row<%=rowN++&1%>>
  <th>SCCP user id</th>
  <td><input class=txt name=sccpUserId value="<%=StringEncoderDecoder.encode(bean.getSccpUserId())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SCCP user ssn</th>
  <td><input class=txt name=sccpUserSsn value="<%=StringEncoderDecoder.encode(bean.getSccpUserSsn())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SCCP MSC GT</th>
  <td><input class=txt name=sccpMscGt value="<%=StringEncoderDecoder.encode(bean.getSccpMscGt())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SCCP VLR GT</th>
  <td><input class=txt name=sccpVlrGt value="<%=StringEncoderDecoder.encode(bean.getSccpVlrGt())%>"></td>
</tr>
  <tr class=row<%=rowN++&1%>>
  <th>SCCP HLR GT</th>
  <td><input class=txt name=sccpHlrGt value="<%=StringEncoderDecoder.encode(bean.getSccpHlrGt())%>"></td>
</tr>
</table><br/>


<div class=page_subtitle><%= getLocString("mtsmsme.subtitle.a_to_a_map")%></div>
<script type="text/javascript">

function clickAddMapping(counter)
{
	var addressElem = document.getElementById('mapping_new_address');
  var regTypeElem   = document.getElementById('mapping_new_reg_type');
  var aliasElem   = document.getElementById('mapping_new_alias');
  var mgtElem   = document.getElementById('mapping_new_mgt');
  var msisdnElem   = document.getElementById('mapping_new_msisdn');
  var periodElem   = document.getElementById('mapping_new_period');
  var commentElem   = document.getElementById('mapping_new_comment');
  var mapping_table_id   = document.getElementById('mapping_table_id');

  var newRow = mapping_table_id.insertRow(mapping_table_id.rows.length-2);
	newRow.className = "row" + (mapping_table_id.rows.length & 1);
	newRow.id = "mapping_row_" + addressElem.value;

  var newHide = document.createElement("td");
  newHide.innerHTML = "<img alt=\"\" src=\"/images/list_closed.gif\" id=\"sectionHeader_alias"+counter+"\" onClick=\"showhide_section('alias"+counter+"')\">";
  newRow.appendChild(newHide);

  var newCell = document.createElement("td");
//  var newSection = addressElem.value.replace(/[.]/g, "_");
  var i = addressElem.value.lastIndexOf('.');
  if (i < 0) i = -1;
  var newSection = addressElem.value.substring(i + 1, addressElem.value.length);
	newCell.innerHTML = "<input class=txtW name=\"<%=Index.MAPPING_SECTION_NAME%>." + newSection + ".address\" value=\"" + addressElem.value + "\">";
	newRow.appendChild(newCell);

  newCell = document.createElement("td");
	newCell.innerHTML = "<select class=selectW  name=\"<%=Index.MAPPING_SECTION_NAME%>." + newSection + ".reg_type\"><option value=\"external\"" + ((regTypeElem.value == "external") ? "SELECTED" : "") + ">external</option><option value=\"internal\"" + ((regTypeElem.value == "internal") ? "SELECTED" : "") + ">internal</option></select>";
	newRow.appendChild(newCell);

  newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txtW name=\"<%=Index.MAPPING_SECTION_NAME%>." + newSection + ".alias\" value=\"" + aliasElem.value + "\">";
	newRow.appendChild(newCell);

  newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txtW name=\"<%=Index.MAPPING_SECTION_NAME%>." + newSection + ".mgt\" value=\"" + mgtElem.value + "\">";
	newRow.appendChild(newCell);

  newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txtW name=\"<%=Index.MAPPING_SECTION_NAME%>." + newSection + ".msisdn\" value=\"" + msisdnElem.value + "\">";
	newRow.appendChild(newCell);

  newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txtW name=\"<%=Index.MAPPING_SECTION_NAME%>." + newSection + ".period\" value=\"" + periodElem.value + "\">";
	newRow.appendChild(newCell);

  newCell = document.createElement("td");
	newCell.innerHTML = "<img src=\"/images/but_del.gif\" class=button jbuttonName=\"mbRemove\" jbuttonValue=\"common.buttons.remove\" title=\"mtsmsme.hint.remove_mapping\" onClick=\"return clickRemoveMapping('" + newRow.id + "');\">";
	newRow.appendChild(newCell);

  var newCommentRow = mapping_table_id.insertRow(mapping_table_id.rows.length-2);
	newCommentRow.id = "sectionValue_alias"+counter;
  newCommentRow.style.display = "none";
	newCommentRow.className = "row" + (mapping_table_id.rows.length & 1);

  newCell = document.createElement("td");
  newCell.innerHTML = "Note";
  newCommentRow.appendChild(newCell);

  newCell = document.createElement("td");
  newCell.colSpan="7";
  newCell.innerHTML = "<input style=\"width:100%\" name=\"<%=Index.MAPPING_SECTION_NAME%>." + newSection + ".comment\" value=\"" + commentElem.value + "\">";
	newCommentRow.appendChild(newCell);

  addressElem.value = "";
	regTypeElem.value = "";
	aliasElem.value = "";
	mgtElem.value = "";
	msisdnElem.value = "";
	periodElem.value = "";
	commentElem.value = "";

	return false;
}
function clickRemoveMapping(id_to_remove)
{
	var rowElem = mapping_table_id.rows[id_to_remove];
	var commElem = mapping_table_id.rows[id_to_remove+1];
  var ind = rowElem.rowIndex;
  mapping_table_id.deleteRow(ind);
	mapping_table_id.deleteRow(ind);

	return false;
}
</script>
<table class=list cellspacing=0 id=mapping_table_id width="100%">
<col width=1%>
<col width=20%>
<col width=10%>
<col width=20%>
<col width=20%>
<col width=20%>
<col width=10%>
<col width=1%>
<tr>
  <th>&nbsp;</th>
  <th>address</th>
	<th>regType</th>
	<th>alias</th>
	<th>mgt</th>
	<th>msisdn</th>
	<th>period</th>
	<th>&nbsp;</th>
</tr>

<%!
  private static int counter = 0;
%>
<%
  rowN = 0;
  Collection mappingSectionNames = bean.getMappingSectionNames();
  for (Iterator i = mappingSectionNames.iterator(); i.hasNext();) {
    String sectionName = "alias"+(counter++);
    final String section = (String) i.next();
		final String address = bean.getString(section + ".address");
		final String comment = bean.getString(section + ".comment");
		final String alias   = bean.getString(section + ".alias");
		final String mgt   = bean.getString(section + ".mgt");
		final String msisdn   = bean.getString(section + ".msisdn");
		final int period   = bean.getInt(section + ".period");
		final String regType = bean.getString(section + ".reg_type");

    %>

  <tr class=row<%=(rowN++)&1%> id=mapping_row_<%=StringEncoderDecoder.encode(address)%>>
			<td><img alt="" src="/images/list_closed.gif" id="sectionHeader_<%=sectionName%>" onclick="showhide_section('<%=sectionName%>')"></td>
			<td><input class=txtW  name="<%=section%>.address" value="<%=address%>"></td>
			<td><select class=selectW  name="<%=section%>.reg_type">
            <option value="external" <%=regType != null && regType.equals("external") ? "SELECTED" : ""%>>external</option>
            <option value="internal" <%=regType != null && regType.equals("internal") ? "SELECTED" : ""%>>internal</option>
			</select></td>
			<td><input class=txtW name="<%=section%>.alias"   value="<%=alias%>"></td>
			<td><input class=txtW name="<%=section%>.mgt"   value="<%=mgt%>"></td>
			<td><input class=txtW name="<%=section%>.msisdn"   value="<%=msisdn%>"></td>
			<td><input class=txtW name="<%=section%>.period"   value="<%=period%>"></td>
			<td><%button(out, "/images/but_del.gif", "mbDel", "common.buttons.remove", "mtsmsme.hint.remove_mapping", "return clickRemoveMapping('mapping_row_" + StringEncoderDecoder.encode(address) + "');");%></td>
	</tr>
  <tr id="sectionValue_<%=sectionName%>" style="display:none">
			<td>Note</td><td colspan="7"><input style="width:100%" name="<%=section%>.comment"   value="<%= comment == null ? "" : comment%>"></td>
  </tr>
  <%
	}
%>
<tr id=mapping_new class=row<%=(rowN+1)&1%>>
  <td>&nbsp;</td>
  <td><input class=txtW  id="mapping_new_address" name="mapping_new_address"></td>
	<td><select class=selectW  id="mapping_new_reg_type" name="mapping_new_reg_type">
            <option value="external">external</option>
            <option value="internal">internal</option>
			</select></td>
	<td><input class=txtW id="mapping_new_alias"   name="mapping_new_alias" ></td>
	<td><input class=txtW id="mapping_new_mgt"   name="mapping_new_mgt"  ></td>
	<td><input class=txtW id="mapping_new_msisdn"   name="mapping_new_msisdn" ></td>
	<td><input class=txtW id="mapping_new_period"   name="mapping_new_period" ></td>
	<td><%button(out, "/images/but_add.gif", "mbAdd", "common.buttons.add", "mtsmsme.hint.add_mapping", "return clickAddMapping('"+(counter++)+"');");%></td>
</tr>
<tr>
			<td>Note</td><td colspan="7"><input style="width:100%" id="mapping_new_comment" name="mapping_new_comment"></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbApply",  "common.buttons.apply",  "mtsmsme.hint.apply");
page_menu_button(session, out, "mbReset",  "common.buttons.reset",  "mtsmsme.hint.reset");
page_menu_space(out);
page_menu_end(out);%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>