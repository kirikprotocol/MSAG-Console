<%@ page import="ru.novosoft.smsc.util.*,
                 java.util.*,
                 ru.novosoft.smsc.admin.acl.AclInfo,
                 ru.novosoft.smsc.admin.route.Route,
                 ru.novosoft.smsc.admin.provider.Provider,
                 ru.novosoft.smsc.admin.category.Category"%>
<div class=content>
<%int rowN = 0;%>
<script>
var global_counter = 0;
function srcSmeIdChanged()
{
  var rowDisabled = opForm.all.srcSmeId.options[opForm.all.srcSmeId.selectedIndex] == null || opForm.all.srcSmeId.options[opForm.all.srcSmeId.selectedIndex].value != 'MAP_PROXY';
  opForm.all.forwardTo_row.disabled = rowDisabled;
  opForm.all.forwardTo.disabled = rowDisabled;
}
</script>
<table cellspacing=0 cellpadding=0 _border=1>
<col width="35%">
<col width="10%" >
<col width="45%">
<tr>
	<td valign=top><%rowN = 0;%>
		<div class=page_subtitle>Route information</div>
		<table cellspacing=0 cellpadding=0>
		<col width="1%">
		<col width="98%">
		<tr class=row<%=(rowN++)&1%>>
			<th>name</th>
			<td><input class=txt name=routeId value="<%=bean.getRouteId()%>"></td>
		</tr>
		<tr class=row<%=(rowN++)&1%>>
			<th>notes</th>
			<td><input class=txt name=notes value="<%=bean.getNotes()%>"></td>
		</tr>
		<tr class=row<%=(rowN++)&1%>>
			<th><label title="integer from 0 to 32767">priority</label></th>
			<td><input class=txt name=priority value="<%=bean.getPriority()%>" maxlength=5 validation="priority" onkeyup="resetValidation(this)"></td>
		</tr>
		<tr class=row<%=(rowN++)&1%>>
			<th>service&nbsp;ID&nbsp;</th>
			<td><input class=txt name=serviceId value="<%=bean.getServiceId()%>" maxlength=5 validation="route_serviceId" onkeyup="resetValidation(this)"></td>
		</tr>
		<tr class=row<%=(rowN++)&1%>>
			<th>delivery&nbsp;mode</th>
			<td><select name=deliveryMode>
				<option value="default"  <%="default" .equalsIgnoreCase(bean.getDeliveryMode()) ? "selected" : ""%>>default</option>
				<option value="store"    <%="store"   .equalsIgnoreCase(bean.getDeliveryMode()) ? "selected" : ""%>>store and forward</option>
				<option value="forward"  <%="forward" .equalsIgnoreCase(bean.getDeliveryMode()) ? "selected" : ""%>>forward</option>
				<option value="datagram" <%="datagram".equalsIgnoreCase(bean.getDeliveryMode()) ? "selected" : ""%>>datagram</option>
			</select></td>
    </tr>
		<tr class=row<%=(rowN++)&1%>>
			<th>source&nbsp;SME&nbsp;ID</th>
			<td><select name=srcSmeId id=srcSmeId onchange="srcSmeIdChanged()">
				<option value="" <%=(bean.getSrcSmeId() == null || bean.getSrcSmeId().length() == 0) ? "selected" : ""%>></option>
				<%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
				{
					String smeId = (String) j.next();
					String encSmeId = StringEncoderDecoder.encode(smeId);
					%><option value="<%=encSmeId%>" <%=smeId.equals(bean.getSrcSmeId()) ? "selected" : ""%>><%=encSmeId%></option><%
				}
				%>
			</select></td>
		</tr>
    <tr class=row<%=(rowN++)&1%>>
			<th>Providers&nbsp;</th>
			<td><select name=providerIdStr id=providerId >
				<option value="" <%=(bean.getProviderId() == -1 ) ? "selected" : ""%>></option>
				<%for (Iterator iprov = bean.getProviders().iterator(); iprov.hasNext(); )
				{
					Provider provider = (Provider) iprov.next();
					String ProviderName=provider.getName();
          long ProviderId=provider.getId();
          String ProviderIdStr=String.valueOf(ProviderId);
          String encProviderName = StringEncoderDecoder.encode(ProviderName);
					%><option value="<%=ProviderIdStr%>" <%=(ProviderId==bean.getProviderId()) ? "selected" : ""%>><%=encProviderName%></option><%
				}
				%>
			</select></td>
		</tr>
    <tr class=row<%=(rowN++)&1%>>
			<th>Categories&nbsp;</th>
			<td><select name=categoryIdStr id=categoryId >
				<option value="" <%=(bean.getCategoryId() == -1) ? "selected" : ""%>></option>
				<%for (Iterator jcat = bean.getCategories().iterator(); jcat.hasNext(); )
				{
				 Category category = (Category) jcat.next();
          long CategoryId=category.getId();
          String CategoryIdStr=String.valueOf(CategoryId);
					String CategoryName =category.getName();
          String encCategoryName = StringEncoderDecoder.encode(CategoryName);
					%><option value="<%=CategoryIdStr%>" <%=(CategoryId==bean.getCategoryId()) ? "selected" : ""%>><%=encCategoryName%></option><%
				}
				%>
			</select></td>
		</tr>
    <tr class=row<%=(rowN++)&1%> id=forwardTo_row>
      <th>forward to</th>
      <td><input id=forwardTo class=txt name=forwardTo value="<%=StringEncoderDecoder.encode(bean.getForwardTo())%>" validation="address" onkeyup="resetValidation(this)"><script>srcSmeIdChanged();</script></td>
    </tr>
    <tr class=row<%=(rowN++)&1%>>
      <th>Access&nbsp;Control&nbsp;List</th>
      <td>
        <div class=select><select class=txt name="aclId">
          <option value="-1">&nbsp;</option>
          <%
            for (Iterator acls = bean.getAclNames().iterator(); acls.hasNext();) {
              AclInfo aclInfo = (AclInfo) acls.next();
              %><option value="<%=aclInfo.getId()%>" <%=aclInfo.getId() == bean.getAclId() ? "selected" : ""%>><%=aclInfo.getName()%></option><%
            }
          %>
        </select></div>
      </td>
    </tr>
		<tr>
      <th>replay path</th>
			<td colspan="2"><select name="replayPath">
        <option value="<%=Route.REPLAY_PATH_PASS%>"     <%=bean.getReplayPath() == Route.REPLAY_PATH_PASS     ? "selected" : ""%>>pass</option>
        <option value="<%=Route.REPLAY_PATH_FORCE%>"    <%=bean.getReplayPath() == Route.REPLAY_PATH_FORCE    ? "selected" : ""%>>force</option>
        <option value="<%=Route.REPLAY_PATH_SUPPRESS%>" <%=bean.getReplayPath() == Route.REPLAY_PATH_SUPPRESS ? "selected" : ""%>>suppress</option>
      </td>
		</tr>
		</table>
	</td>
	<td>&nbsp;</td>
	<td valign=top><%rowN = 0;%>
		<div class=page_subtitle>Route options</div>
		<table cellspacing=2 cellpadding=0>
		<col width=1%>
		<col width=99%>
		<tr>
			<td><input id=active class=check type=checkbox name=active <%=bean.isActive() ? "checked" : ""%>></td>
			<td><label for=active style="padding-left:4px;">active</label></td>
		</tr>
		<tr>
			<td><input id=permissible class=check type=checkbox name=permissible <%=bean.isPermissible() ? "checked" : ""%>></td>
			<td><label for=permissible style="padding-left:4px;">allowed</label></td>
		</tr>
		<tr>
			<td><input class=check type=checkbox id=billing name=billing <%=bean.isBilling() ? "checked" : ""%>></td>
			<td><label for=billing style="padding-left:4px;">billing</label></td>
		</tr>
		<tr>
			<td><input id=archiving class=check type=checkbox name=archiving <%=bean.isArchiving() ? "checked" : ""%>></td>
			<td><label for=archiving style="padding-left:4px;">archiving</label></td>
		</tr>
		<tr>
			<td><input id=suppressDeliveryReports class=check type=checkbox name=suppressDeliveryReports <%=bean.isSuppressDeliveryReports() ? "checked" : ""%>></td>
			<td><label for=suppressDeliveryReports style="padding-left:4px;">suppress delivery reports</label></td>
		</tr>
		<tr>
			<td><input id=hide class=check type=checkbox name=hide <%=bean.isHide() ? "checked" : ""%>></td>
			<td><label for=hide style="padding-left:4px;">hide</label></td>
		</tr>
		<tr>
			<td><input id=forceDelivery class=check type=checkbox name=forceDelivery <%=bean.isForceDelivery() ? "checked" : ""%>></td>
			<td><label for=forceDelivery style="padding-left:4px;">force delivery</label></td>
		</tr>
		<tr>
			<td><input id="allowBlocked" class=check type=checkbox name="allowBlocked" <%=bean.isAllowBlocked() ? "checked" : ""%>></td>
			<td><label for=allowBlocked style="padding-left:4px;">allow blocked</label></td>
		</tr>
		</table>
	</td>
</tr>
</table>
<br>
<hr>
<table cellspacing=0 cellpadding=0 _border=1>
<col width="45%">
<col width="10%" >
<col width="45%">
<tr>
	<td valign=top><%rowN = 0;%>
		<div class=page_subtitle>Sources</div>
		<table cellspacing=0 cellpadding=0>
		<col width="50%" align=left>
		<col width="50%" align=right>
		<col width="0%" align=left>
		<!--col width="50%" align=right>
		<col width="0%"-->
		<tr valign="middle">
			<td>Subject</td>
			<td><select id=srcSubjSelect name="fake_name" class="txt" ><%
				for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
				{
					String name = (String) i.next();
					if (!bean.isSrcChecked(name)) {
						String encName = StringEncoderDecoder.encode(name);
						String hexName = StringEncoderDecoder.encodeHEX(name);
						%><option value="<%=encName%>"><%=encName%></option><%
					}
				}%></select></td>
			<td><img src="/images/but_add.gif" onclick="addSourceSubj()" style="cursor:hand;"></td>
		</tr><tr>
			<td>Mask</td>
			<td><input id=newSrcMask class=txt name=srcMasks validation="routeMask" onkeyup="resetValidation(this)"></td>
			<td><img src="/images/but_add.gif" onclick="addSourceMask(opForm.all.newSrcMask)" style="cursor:hand;"></td>
		</tr>
		</table>
	</td>
	<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>
	<td valign=top>
		<div class=page_subtitle>Destinations</div>
		<table cellspacing=0 cellpadding=0>
		<col width="89%">
		<col width="10%" align=right>
		<col width="1%">
		<col width="0%">
		<tr>
			<td>Subject</td>
			<td><select id=dstSubjSelect onchange="return selectDefaultSme();"><%
				for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
				{
					String name = (String) i.next();
					if (!bean.isDstChecked(name)) {
						String encName = StringEncoderDecoder.encode(name);
						%><option value="<%=encName%>" defaultSme="<%=StringEncoderDecoder.encode(bean.getDefaultSubjectSme(name))%>"><%=encName%></option><%
					}
				}%></select></td>
			<td><select id=dstSubjSmeSelect><%
				for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
				{
					String smeId = (String) j.next();
					String encSmeId = StringEncoderDecoder.encode(smeId);
					%><option id="<%=encSmeId%>" value="<%=encSmeId%>"><%=encSmeId%></option><%
				}
				%></select></td>
			<td><img src="/images/but_add.gif" onclick="addDestSubj()" style="cursor:hand;"></td>
		</tr>
		<tr>
			<td>Mask</td>
			<td><input id=newDstMask class=txt name=dstMasks validation="routeMask" onkeyup="resetValidation(this)"></td>
			<td><select name=dst_mask_sme_ id=newDstMaskSme>
				<%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
				{
					String smeId = (String) j.next();
					String encSmeId = StringEncoderDecoder.encode(smeId);
					%><option value="<%=encSmeId%>" <%=smeId.equals(bean.getDst_mask_sme_()) ? "selected" : ""%>><%=encSmeId%></option><%
				}
				%>
				</select>
			</td>
			<td><img src="/images/but_add.gif" onclick="addDestMask()" style="cursor:hand;"></td>
		</tr>
		</table>
	</td>
</tr>
<tr>
	<td valign=top>
		<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
		<script>
			function addSourceMask(valueElem) {
				if (validateField(valueElem)) {
					var tbl = opForm.all.sources_table;
					var newRow = tbl.insertRow(tbl.rows.length);
					newRow.className = "row" + ((tbl.rows.length+1) & 1);
					newRow.id = "srcRow_" + (global_counter++);
					newCell = document.createElement("td");
					newCell.innerHTML = '<img src="/images/mask.gif">';
					newRow.appendChild(newCell);
					newCell = document.createElement("td");
					newCell.innerHTML = valueElem.value + '<input type=hidden name=srcMasks value="' + valueElem.value + '">';
					newRow.appendChild(newCell);
					newCell = document.createElement("td");
					newCell.innerHTML = '<img src="/images/but_del.gif" onClick="removeRow(opForm.all.sources_table, \'' + newRow.id + '\')" style="cursor: hand;">';
					newRow.appendChild(newCell);
					valueElem.value = "";
					valueElem.focus();
					return true;
				} else 
					return false;
			}
			function removeSrcSubj(rowId)
			{
				var selectElem = opForm.all.srcSubjSelect;
				var tbl = opForm.all.sources_table;
				var rowElem = tbl.rows(rowId);
				var subjValue = rowElem.all.subjSrc.value;
				var oOption = document.createElement("OPTION");
				selectElem.options.add(oOption);
				oOption.innerText = subjValue;
				oOption.value = subjValue;
				selectElem.disabled = false;
				tbl.deleteRow(rowElem.rowIndex);
			}
			function addSourceSubj() {
				var selectElem = opForm.all.srcSubjSelect;
				if (selectElem.options.length > 0) {
					var subjValue = selectElem.options[selectElem.selectedIndex].value;
					var tbl = opForm.all.sources_table;
					var newRow = tbl.insertRow(tbl.rows.length);
					newRow.className = "row" + ((tbl.rows.length+1) & 1);
					newRow.id = "srcRow_" + (global_counter++);
					newCell = document.createElement("td");
					newCell.innerHTML = '<img src="/images/subject.gif">';
					newRow.appendChild(newCell);
					newCell = document.createElement("td");
					newCell.innerHTML = subjValue + '<input id=subjSrc type=hidden name=checkedSources value="' + subjValue + '">';
					newRow.appendChild(newCell);
					newCell = document.createElement("td");
					newCell.innerHTML = '<img src="/images/but_del.gif" onClick="removeSrcSubj(\'' + newRow.id + '\');" style="cursor: hand;">';
					newRow.appendChild(newCell);
					selectElem.options[selectElem.selectedIndex] = null;
					selectElem.focus();
					if (selectElem.options.length == 0)
						selectElem.disabled = true;
				}
			}
		</script>
		<hr>
		<table id=sources_table class=properties_list cellspacing=0 cellpadding=0>
		<col width="1%">
		<col width="100%">
		<%
		for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
		{
			String name = (String) i.next();
			String encName = StringEncoderDecoder.encode(name);
			String rowId = "subjRow_" + StringEncoderDecoder.encodeHEX(name);
			if (bean.isSrcChecked(name)) {%>
				<tr class=row<%=(rowN++)&1%> id="<%=rowId%>">
					<td><img src="/images/subject.gif"></td>
					<td><%=encName%><input id=subjSrc type=hidden name=checkedSources value="<%=encName%>"></td>
					<td><img src="/images/but_del.gif" onClick="removeSrcSubj('<%=rowId%>');" style="cursor: hand;"></td>
				</tr><%
			}
		}
		for (int i=0; i<bean.getSrcMasks().length; i++)
		{
			String rowId = "maskRow_" + StringEncoderDecoder.encodeHEX(bean.getSrcMasks()[i]);
			%>
			<tr class=row<%=(rowN++)&1%> id=<%=rowId%>>
				<td><img src="/images/mask.gif"></td>
				<td><%=bean.getSrcMasks()[i]%><input type=hidden name=srcMasks value="<%=bean.getSrcMasks()[i]%>"></td>
				<td><img src="/images/but_del.gif" onClick="removeRow(opForm.all.sources_table, '<%=rowId%>')" style="cursor: hand;"></td>
			</tr><%
		}%>
		</table>
	</td>
	<td>&nbsp;</td>
	<td valign=top><%rowN = 0;%>
		<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Destinations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
		<script>
			var smesSelectText = "<select name=fake_dst_mask_sme_ id=newSmesSelect><%
				for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
				{
					String smeId = (String) j.next();
					String encSmeId = StringEncoderDecoder.encode(smeId);
					String hexSmeId = StringEncoderDecoder.encodeHEX(smeId);
					%><option value=\"<%=encSmeId%>\" id=option_<%=hexSmeId%>><%=encSmeId%></option><%
				}
				%></select>";
				
			function addDestMask() {
				if (validateField(opForm.all.newDstMask)) {
					var mask = opForm.all.newDstMask.value;
					var smeSelect = opForm.all.newDstMaskSme;
					var sme = smeSelect.options[smeSelect.selectedIndex].value;
					var tbl = opForm.all.destinations_table;
					
					var newRow = tbl.insertRow(tbl.rows.length);
					newRow.className = "row" + ((tbl.rows.length+1) & 1);
					newRow.id = "srcRow_" + (global_counter++);
					
					newCell = document.createElement("td");
					newCell.innerHTML = '<img src="/images/mask.gif">';
					newRow.appendChild(newCell);
					
					newCell = document.createElement("td");
					newCell.innerHTML = mask + '<input type=hidden name=dstMasks value="' + mask + '">';
					newRow.appendChild(newCell);
					
					newCell = document.createElement("td");
					newCell.innerHTML = smesSelectText;
					newSelect = newCell.all.newSmesSelect;
					newSelect.name = "dst_mask_sme_" + encodeHEX(mask);					
					newSelect.all["option_" + encodeHEX(sme)].selected = true;
					newRow.appendChild(newCell);
					
					newCell = document.createElement("td");
					newCell.innerHTML = '<img src="/images/but_del.gif" onClick="removeRow(opForm.all.destinations_table, \'' + newRow.id + '\')" style="cursor: hand;">';
					newRow.appendChild(newCell);
					opForm.all.newDstMask.value = "";
					opForm.all.newDstMask.focus();
					return true;
				} else
					return false;
			}
      function selectDefaultSme(){
        var selectElem = opForm.all.dstSubjSelect;
        var smeSelectElem = opForm.all.dstSubjSmeSelect;
        if (selectElem.options.length > 0)
        {
          var defaultSme = selectElem.options[selectElem.selectedIndex].defaultSme;
          if (defaultSme != null) {
            var optionElem = smeSelectElem.options(defaultSme, 0);
            if (optionElem != null)
            optionElem.selected = true;
          }
        }
        return true;
      }
			function removeDestSubj(rowId)
			{
				var selectElem = opForm.all.dstSubjSelect;
				var tbl = opForm.all.destinations_table;
				var rowElem = tbl.rows(rowId);
				var subjValue = rowElem.all.subjDst.value;
        var subjDefaultSme = rowElem.all.subjDst.defaultSme;
				var oOption = document.createElement("OPTION");
				selectElem.options.add(oOption);
				oOption.innerText = subjValue;
				oOption.value = subjValue;
        oOption.defaultSme = subjDefaultSme;
				selectElem.disabled = false;
				tbl.deleteRow(rowElem.rowIndex);
        return selectDefaultSme();
			}
			function addDestSubj() {
				var selectElem = opForm.all.dstSubjSelect;
				if (selectElem.options.length > 0) {
					var subjValue = selectElem.options[selectElem.selectedIndex].value;
          var subjDefaultSme = selectElem.options[selectElem.selectedIndex].defaultSme;
					var tbl = opForm.all.destinations_table;
					var smeSelect = opForm.all.dstSubjSmeSelect;
					var sme = smeSelect.options[smeSelect.selectedIndex].value;
					var newRow = tbl.insertRow(tbl.rows.length);
					newRow.className = "row" + ((tbl.rows.length+1) & 1);
					newRow.id = "srcRow_" + (global_counter++);

					newCell = document.createElement("td");
					newCell.innerHTML = '<img src="/images/subject.gif">';
					newRow.appendChild(newCell);

					newCell = document.createElement("td");
					newCell.innerHTML = subjValue + '<input id=subjDst type=hidden name=checkedDestinations value="' + subjValue + '">';
          newCell.all.subjDst.defaultSme = subjDefaultSme;
					newRow.appendChild(newCell);

					newCell = document.createElement("td");
					newCell.innerHTML = smesSelectText;
					newSelect = newCell.all.newSmesSelect;
					newSelect.name = "dst_sme_" + encodeHEX(subjValue);
					newSelect.all["option_" + encodeHEX(sme)].selected = true;
					newRow.appendChild(newCell);

					newCell = document.createElement("td");
					newCell.innerHTML = '<img src="/images/but_del.gif" onClick="removeDestSubj(\'' + newRow.id + '\');" style="cursor: hand;">';
					newRow.appendChild(newCell);
					selectElem.options[selectElem.selectedIndex] = null;
					selectElem.focus();
					if (selectElem.options.length == 0)
						selectElem.disabled = true;

          return selectDefaultSme();
				}
			}
		</script>
		<hr>
		<table class=properties_list cellspacing=0 cellpadding=0 id=destinations_table>
		<col width="1%">
		<col width="99%">
		<col width="1%">
		<col width="1%">
		<%rowN=0;
		for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
		{
			String name = (String) i.next();
			if (bean.isDstChecked(name)) {
				String encName = StringEncoderDecoder.encode(name);
				String hexName = StringEncoderDecoder.encodeHEX(name);
				String rowId = "subjRow_" + StringEncoderDecoder.encodeHEX(name);
				%><tr class=row<%=(rowN++)&1%> id=<%=rowId%>>
					<td><img src="/images/subject.gif"></td>
					<td><%=encName%><input id=subjDst type=hidden name=checkedDestinations value="<%=encName%>" defaultSme="<%=StringEncoderDecoder.encode(bean.getDefaultSubjectSme(name))%>"></td>
					<td><select name=dst_sme_<%=hexName%>>
						<%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
						{
							String smeId = (String) j.next();
							String encSmeId = StringEncoderDecoder.encode(smeId);
							%><option value="<%=encSmeId%>" <%=bean.isSmeSelected(name, smeId) ? "selected" : ""%>><%=encSmeId%></option><%
						}
						%>
						</select>
					</td>
					<td><img src="/images/but_del.gif" onClick="removeDestSubj('<%=rowId%>');" style="cursor: hand;"></td>
				</tr><%
			}
		}
		for (int i=0; i<bean.getDstMasks().length; i++)
		{
		String dstMask = bean.getDstMasks()[i];
		String encMask = StringEncoderDecoder.encode(dstMask);
		String hexMask = StringEncoderDecoder.encodeHEX(dstMask);
		String rowId = "maskRow_" + StringEncoderDecoder.encodeHEX(dstMask);
		%>
		<tr class=row<%=(rowN++)&1%> id="<%=rowId%>">
			<td><img src="/images/mask.gif"></td>
			<td><%=encMask%><input type=hidden name=dstMasks value="<%=encMask%>"></td>
			<td><select name=dst_mask_sme_<%=hexMask%> onkeyup="resetValidation(this)">
				<%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
				{
					String smeId = (String) j.next();
					String encSmeId = StringEncoderDecoder.encode(smeId);
					%><option value="<%=encSmeId%>" <%=bean.isMaskSmeSelected(dstMask, smeId) ? "selected" : ""%>><%=encSmeId%></option><%
				}
				%>
				</select>
			</td>
			<td><img src="/images/but_del.gif" onClick="removeRow(opForm.all.destinations_table, '<%=rowId%>')" style="cursor: hand;"></td>
		</tr>
		<%}%>
		</table>
	</td>
</tr>
</table>
<script>
selectDefaultSme();
</script>
</div>