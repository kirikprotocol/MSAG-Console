<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.mtsmsme.beans.Index,
                 java.util.*, ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.util.Functions"%>
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
    document.getElementById('mbStart').disabled = (status == "<%= getLocString("common.statuses.online1") %>" ||
                                     status == "<%= getLocString("common.statuses.online2") %>" ||
                                     status == "<%= getLocString("common.statuses.running") %>" ||
                                     status == "<%= getLocString("common.statuses.stopping")%>" ||
                                     status == "<%= getLocString("common.statuses.starting")%>" ||
                                     status == "<%= getLocString("common.statuses.unknown" )%>" );
    document.getElementById('mbStop').disabled  = (status == "<%= getLocString("common.statuses.offline") %>" ||
                                     status == "<%= getLocString("common.statuses.stopped") %>" ||
                                     status == "<%= getLocString("common.statuses.stopping")%>" ||
                                     status == "<%= getLocString("common.statuses.unknown") %>" );
    window.setTimeout(checkStartStop, 500);
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
<div class=page_subtitle><%= getLocString("mtsmsme.subtitle.a_to_a_map")%></div>
<script type="text/javascript">
function clickAddMapping()
{
	var addressElem = document.getElementById('mapping_new_address');
    var aliasElem   = document.getElementById('mapping_new_alias');

    var newRow = mapping_table_id.insertRow(mapping_table_id.rows.length-1);
	newRow.className = "row" + (mapping_table_id.rows.length & 1);
	newRow.id = "mapping_row_" + addressElem.value;
	var newCell = document.createElement("td");
    var newSection = addressElem.value.replace(/[.]/g, "_");

	newCell.innerHTML = "<input class=txt name=\"<%=Index.MAPPING_SECTION_NAME%>." + newSection + ".address\" value=\"" + addressElem.value + "\">";
	newRow.appendChild(newCell);
	newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txtW name=\"<%=Index.MAPPING_SECTION_NAME%>." + newSection + ".alias\" value=\"" + aliasElem.value + "\">";
	newRow.appendChild(newCell);
	newCell = document.createElement("td");
	newCell.innerHTML = "<img src=\"/images/but_del.gif\" class=button jbuttonName=\"mbRemove\" jbuttonValue=\"common.buttons.remove\" title=\"mtsmsme.hint.remove_mapping\" jbuttonOnclick=\"return clickRemoveMapping('" + newRow.id + "');\">";
	newRow.appendChild(newCell);

	addressElem.value = "";
	aliasElem.value = "";

	return false;
}
function clickRemoveMapping(id_to_remove)
{
	var rowElem = mapping_table_id.rows(id_to_remove);
	mapping_table_id.deleteRow(rowElem.rowIndex);

	return false;
}
</script>
<table class=list cellspacing=0 id=mapping_table_id>
<col width=1%>
<col width=98%>
<col width=1%>
<tr>
	<th>address</th>
	<th colspan=2>alias</th>
</tr>
<%rowN = 0;
	Collection mappingSectionNames = bean.getMappingSectionNames();
	for (Iterator i = mappingSectionNames.iterator(); i.hasNext();) {
    final String section = (String) i.next();
		final String address = bean.getString(section + ".address");
		final String alias   = bean.getString(section + ".alias");
		%><tr class=row<%=(rowN++)&1%> id=mapping_row_<%=StringEncoderDecoder.encode(address)%>>
			<td><input class=txt  name="<%=section%>.address" value="<%=address%>"></td>
			<td><input class=txtW name="<%=section%>.alias"   value="<%=alias%>"></td>
			<td><%button(out, "/images/but_del.gif", "mbDel", "common.buttons.remove", "mtsmsme.hint.remove_mapping", "return clickRemoveMapping('mapping_row_" + StringEncoderDecoder.encode(address) + "');");%></td>
		</tr><%
	}
%>
<tr id=mapping_new class=row<%=(rowN+1)&1%>>
	<td><input class=txt  id="mapping_new_address" name="mapping_new_address"></td>
	<td><input class=txtW id="mapping_new_alias"   name="mapping_new_alias"  ></td>
	<td><%button(out, "/images/but_add.gif", "mbAdd", "common.buttons.add", "mtsmsme.hint.add_mapping", "return clickAddMapping();");%></td>
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