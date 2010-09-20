<%@ page import="ru.novosoft.smsc.admin.acl.AclInfo,
                 ru.novosoft.smsc.admin.category.Category,
                 ru.novosoft.smsc.admin.provider.Provider,
                 ru.novosoft.smsc.admin.route.Route,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.Iterator" %>
<div class=content>
<%int rowN = 0;%>
<script>
    var global_counter = 0;
    function srcSmeIdChanged()
    {
        var rowDisabled = document.getElementById('srcSmeId').options[document.getElementById('srcSmeId').selectedIndex] == null || document.getElementById('srcSmeId').options[document.getElementById('srcSmeId').selectedIndex].value != 'MAP_PROXY';
        document.getElementById('forwardTo_row').disabled = rowDisabled;
        document.getElementById('forwardTo').disabled = rowDisabled;
    }
</script>
<table cellspacing=0 cellpadding=0 _border=1>
<col width="35%">
<col width="10%">
<col width="45%">
<tr>
<td valign=top><%rowN = 0;%>
<div class=page_subtitle><%=getLocString("routes.subTitle")%></div>
<table cellspacing=0 cellpadding=0>
    <col width="1%">
    <col width="98%">
    <tr class=row<%=(rowN++) & 1%>>
        <th><%=getLocString("routes.name")%></th>
        <td><input class=txt name=routeId value="<%=bean.getRouteId()%>"></td>
    </tr>
    <tr class=row<%=(rowN++) & 1%>>
        <th><%=getLocString("routes.notes")%></th>
        <td><input class=txt name=notes value="<%=bean.getNotes()%>"></td>
    </tr>
    <tr class=row<%=(rowN++) & 1%>>
        <th><label title="<%=getLocString("common.hints.priority")%>"><%=getLocString("routes.priority")%></label></th>
        <td><input class=txt name=priority value="<%=bean.getPriority()%>" maxlength=5 validation="priority"
                   onkeyup="resetValidation(this)"></td>
    </tr>
    <tr class=row<%=(rowN++) & 1%>>
        <th><%=getLocString("routes.serviceId")%></th>
        <td><input class=txt name=serviceId value="<%=bean.getServiceId()%>" maxlength=5 validation="route_serviceId"
                   onkeyup="resetValidation(this)"></td>
    </tr>
    <tr class=row<%=(rowN++) & 1%>>
        <th><%=getLocString("routes.deliveryMode")%></th>
        <td><select name=deliveryMode>
            <option value="default"  <%="default" .equalsIgnoreCase(bean.getDeliveryMode()) ? "selected" : ""%>>
                default</option>
            <option value="store"    <%="store"   .equalsIgnoreCase(bean.getDeliveryMode()) ? "selected" : ""%>>store
                and forward</option>
            <option value="forward"  <%="forward" .equalsIgnoreCase(bean.getDeliveryMode()) ? "selected" : ""%>>
                forward</option>
            <option value="datagram" <%="datagram".equalsIgnoreCase(bean.getDeliveryMode()) ? "selected" : ""%>>
                datagram</option>
        </select></td>
    </tr>
    <tr class=row<%=(rowN++) & 1%>>
        <th><%=getLocString("routes.sourceSmeId")%></th>
        <td><select name=srcSmeId id=srcSmeId onchange="srcSmeIdChanged()">
            <option value="" <%=(bean.getSrcSmeId() == null || bean.getSrcSmeId().length() == 0) ? "selected" : ""%>></option>
            <%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext();) {
                String smeId = (String) j.next();
                String encSmeId = StringEncoderDecoder.encode(smeId);
            %><option
                value="<%=encSmeId%>" <%=smeId.equals(bean.getSrcSmeId()) ? "selected" : ""%>><%=encSmeId%></option><%
            }
        %>
        </select></td>
    </tr>
    <tr class=row<%=(rowN++) & 1%>>
        <th><%=getLocString("routes.provider")%></th>
        <td><select name=providerIdStr id=providerId>
            <option value="" <%=(bean.getProviderId() == -1) ? "selected" : ""%>></option>
            <%for (Iterator iprov = bean.getProviders().iterator(); iprov.hasNext();) {
                Provider provider = (Provider) iprov.next();
                String ProviderName = provider.getName();
                long ProviderId = provider.getId();
                String ProviderIdStr = String.valueOf(ProviderId);
                String encProviderName = StringEncoderDecoder.encode(ProviderName);
            %><option
                value="<%=ProviderIdStr%>" <%=(ProviderId == bean.getProviderId()) ? "selected" : ""%>><%=encProviderName%></option><%
            }
        %>
        </select></td>
    </tr>
    <tr class=row<%=(rowN++) & 1%>>
        <th><%=getLocString("routes.category")%></th>
        <td><select name=categoryIdStr id=categoryId>
            <option value="" <%=(bean.getCategoryId() == -1) ? "selected" : ""%>></option>
            <%for (Iterator jcat = bean.getCategories().iterator(); jcat.hasNext();) {
                Category category = (Category) jcat.next();
                long CategoryId = category.getId();
                String CategoryIdStr = String.valueOf(CategoryId);
                String CategoryName = category.getName();
                String encCategoryName = StringEncoderDecoder.encode(CategoryName);
            %><option
                value="<%=CategoryIdStr%>" <%=(CategoryId == bean.getCategoryId()) ? "selected" : ""%>><%=encCategoryName%></option><%
            }
        %>
        </select></td>
    </tr>
    <tr class=row<%=(rowN++) & 1%> id=forwardTo_row>
        <th><%=getLocString("routes.forwardTo")%></th>
        <td><input id=forwardTo class=txt name=forwardTo value="<%=StringEncoderDecoder.encode(bean.getForwardTo())%>"
                   validation="address" onkeyup="resetValidation(this)"><script>srcSmeIdChanged();</script></td>
    </tr>
    <tr class=row<%=(rowN++) & 1%>>
        <th><%=getLocString("routes.acl")%></th>
        <td>
            <div class=select><select class=txt name="aclId">
                <option value="-1">&nbsp;</option>
                <%
                    for (Iterator acls = bean.getAclNames().iterator(); acls.hasNext();) {
                        AclInfo aclInfo = (AclInfo) acls.next();
                %><option
                    value="<%=aclInfo.getId()%>" <%=aclInfo.getId() == bean.getAclId() ? "selected" : ""%>><%=aclInfo.getName()%></option><%
                }
            %>
            </select></div>
        </td>
    </tr>
    <tr>
        <th><%=getLocString("routes.replayPath")%></th>
        <td colspan="2"><select name="replayPath">
            <option value="<%=Route.REPLAY_PATH_PASS%>"     <%=bean.getReplayPath() == Route.REPLAY_PATH_PASS ? "selected" : ""%>>
                pass</option>
            <option value="<%=Route.REPLAY_PATH_FORCE%>"    <%=bean.getReplayPath() == Route.REPLAY_PATH_FORCE ? "selected" : ""%>>
                force</option>
            <option value="<%=Route.REPLAY_PATH_SUPPRESS%>" <%=bean.getReplayPath() == Route.REPLAY_PATH_SUPPRESS ? "selected" : ""%>>
                suppress</option>
        </select></td>
    </tr>
    <tr>
        <th><%=getLocString("routes.backupSme")%></th>
        <td><select name="backupSmeId" id="backupSmeId">
            <option value="" <%=(bean.getBackupSmeId() == null || bean.getBackupSmeId().length() == 0) ? "selected" : ""%>></option>
            <%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext();) {
                String smeId = (String) j.next();
                String encSmeId = StringEncoderDecoder.encode(smeId);
            %><option
                value="<%=encSmeId%>" <%=smeId.equals(bean.getBackupSmeId()) ? "selected" : ""%>><%=encSmeId%></option><%
            }
        %>
        </select></td>
    </tr>

</table>
</td>
<td>&nbsp;</td>
<td valign=top><%rowN = 0;%>
    <div class=page_subtitle><%=getLocString("routes.optionsTitle")%></div>
    <table cellspacing=2 cellpadding=0>
        <col width=1%>
        <col width=99%>
        <tr>
            <td><input id=active class=check type=checkbox name=active <%=bean.isActive() ? "checked" : ""%>></td>
            <td><label for=active style="padding-left:4px;"><%=getLocString("routes.active")%></label></td>
        </tr>
        <tr>
            <td>
              <select name="trafficMode" id="trafficMode">
                <option value="0" <%=bean.getTrafficMode() == 0 ? "selected" : ""%>><%=getLocString("routes.tm.prohibited")%></option>
                <option value="0" <%=bean.getTrafficMode() == 1 ? "selected" : ""%>><%=getLocString("routes.tm.smsOnly")%></option>
                <option value="0" <%=bean.getTrafficMode() == 2 ? "selected" : ""%>><%=getLocString("routes.tm.ussdOnly")%></option>
                <option value="0" <%=bean.getTrafficMode() == 3 ? "selected" : ""%>><%=getLocString("routes.tm.all")%></option>
              </select>
            </td>
            <td><label for=permissible style="padding-left:4px;"><%=getLocString("routes.tm")%></label></td>
        </tr>
        <tr>
            <td><input id=archiving class=check type=checkbox name=archiving <%=bean.isArchiving() ? "checked" : ""%>>
            </td>
            <td><label for=archiving style="padding-left:4px;"><%=getLocString("routes.archiving")%></label></td>
        </tr>
        <tr>
            <td><input id=suppressDeliveryReports class=check type=checkbox
                       name=suppressDeliveryReports <%=bean.isSuppressDeliveryReports() ? "checked" : ""%>></td>
            <td><label for=suppressDeliveryReports
                       style="padding-left:4px;"><%=getLocString("routes.suppressDeliveryReports")%></label></td>
        </tr>
        <tr>
            <td><input id=hide class=check type=checkbox name=hide <%=bean.isHide() ? "checked" : ""%>></td>
            <td><label for=hide style="padding-left:4px;"><%=getLocString("routes.hide")%></label></td>
        </tr>
        <tr>
            <td><input id=forceDelivery class=check type=checkbox
                       name=forceDelivery <%=bean.isForceDelivery() ? "checked" : ""%>></td>
            <td><label for=forceDelivery style="padding-left:4px;"><%=getLocString("routes.forceDelivery")%></label>
            </td>
        </tr>
        <tr>
            <td><input id="allowBlocked" class=check type=checkbox
                       name="allowBlocked" <%=bean.isAllowBlocked() ? "checked" : ""%>></td>
            <td><label for=allowBlocked style="padding-left:4px;"><%=getLocString("routes.allowBlocked")%></label></td>
        </tr>
        <tr>
            <td><input class=check type=checkbox id=transit name=transit <%=bean.isTransit() ? "checked" : ""%>></td>
            <td><label for=billing style="padding-left:4px;"><%=getLocString("routes.transit")%></label></td>
        </tr>
        <tr>
          <td>
            <select id=billing name=billing>
              <option value=<%=Route.BILLING_TRUE%> <%=bean.getBilling() == Route.BILLING_TRUE ? "SELECTED" : ""%>>on</option>
              <option value=<%=Route.BILLING_FALSE%> <%=bean.getBilling() == Route.BILLING_FALSE ? "SELECTED" : ""%>>off</option>
              <option value=<%=Route.BILLING_MT%> <%=bean.getBilling() == Route.BILLING_MT ? "SELECTED" : ""%>>MT</option>
              <option value=<%=Route.BILLING_FR%> <%=bean.getBilling() == Route.BILLING_FR ? "SELECTED" : ""%>>FR</option>
              <option value=<%=Route.BILLING_ON_SUBMIT%> <%=bean.getBilling() == Route.BILLING_ON_SUBMIT ? "SELECTED" : ""%>>on submit</option>
              <option value=<%=Route.BILLING_CDR%> <%=bean.getBilling() == Route.BILLING_CDR ? "SELECTED" : ""%>>CDR</option>
            </select>
          </td>
          <td><label for=billing style="padding-left:4px;"><%=getLocString("routes.billing")%></label></td>
        </tr>
    </table>
</td>
</tr>
</table>
<br>
<hr>
<table cellspacing=0 cellpadding=0 _border=1>
<col width="45%">
<col width="10%">
<col width="45%">
<tr>
    <td valign=top><%rowN = 0;%>
        <div class=page_subtitle><%=getLocString("common.titles.sources")%></div>
        <table cellspacing=0 cellpadding=0>
            <col width="50%" align=left>
            <col width="50%" align=right>
            <col width="0%" align=left>
            <!--col width="50%" align=right>
           <col width="0%"-->
            <tr valign="middle">
                <td><%=getLocString("common.util.Subject")%></td>
                <td align=RIGHT><select id=srcSubjSelect name="fake_name" class="txt"><%
                    for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();) {
                        String name = (String) i.next();
                        if (!bean.isSrcChecked(name)) {
                            String encName = StringEncoderDecoder.encode(name);
                            String hexName = StringEncoderDecoder.encodeHEX(name);
                %><option value="<%=encName%>"><%=encName%></option><%
                        }
                    }%></select></td>
                <td><img class=button src="/images/but_add.gif" onclick="addSourceSubj()"></td>
            </tr><tr>
            <td><%=getLocString("common.util.Mask")%></td>
            <td><input id=newSrcMask class=txt name=srcMasks validation="routeMask" onkeyup="resetValidation(this)">
            </td>
            <td><img class=button src="/images/but_add.gif" onclick="addSourceMask(document.getElementById('newSrcMask'))"></td>
        </tr>
        </table>
    </td>
    <td>&nbsp;&nbsp;&nbsp;&nbsp;</td>
    <td valign=top>
        <div class=page_subtitle><%=getLocString("common.titles.destinations")%></div>
        <table cellspacing=0 cellpadding=0>
            <col width="89%">
            <col width="10%" align=right>
            <col width="1%">
            <col width="0%">
            <tr>
                <td><%=getLocString("common.util.Subject")%></td>
                <td align=RIGHT><select id=dstSubjSelect onchange="return selectDefaultSme();"><%
                    for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();) {
                        String name = (String) i.next();
                        if (!bean.isDstChecked(name)) {
                            String encName = StringEncoderDecoder.encode(name);
                %><option value="<%=encName%>"
                          defaultSme="<%=StringEncoderDecoder.encode(bean.getDefaultSubjectSme(name))%>"><%=encName%></option><%
                        }
                    }%></select></td>
                <td><select id=dstSubjSmeSelect><%
                    for (Iterator j = bean.getAllSmes().iterator(); j.hasNext();) {
                        String smeId = (String) j.next();
                        String encSmeId = StringEncoderDecoder.encode(smeId);
                %><option id="<%=encSmeId%>" value="<%=encSmeId%>"><%=encSmeId%></option><%
                    }
                %></select></td>
                <td><img class=button src="/images/but_add.gif" onclick="addDestSubj()"></td>
            </tr>
            <tr>
                <td><%=getLocString("common.util.Mask")%></td>
                <td><input id=newDstMask class=txt name=dstMasks validation="routeMask" onkeyup="resetValidation(this)">
                </td>
                <td><select name=dst_mask_sme_ id=newDstMaskSme>
                    <%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext();) {
                        String smeId = (String) j.next();
                        String encSmeId = StringEncoderDecoder.encode(smeId);
                    %><option
                        value="<%=encSmeId%>" <%=smeId.equals(bean.getDst_mask_sme_()) ? "selected" : ""%>><%=encSmeId%></option><%
                    }
                %>
                </select>
                </td>
                <td><img class=button src="/images/but_add.gif" onclick="addDestMask()"></td>
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
                var tbl = document.getElementById('sources_table');
                var newRow = tbl.insertRow(tbl.rows.length);
                newRow.className = "row" + ((tbl.rows.length + 1) & 1);
                newRow.id = "srcRow_" + (global_counter++);
                newCell = document.createElement("td");
                newCell.innerHTML = '<img src="/images/mask.gif">';
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = valueElem.value + '<input type=hidden name=srcMasks value="' + valueElem.value + '">';
                newRow.appendChild(newCell);
                newCell = document.createElement("td");
                newCell.innerHTML = '<img class=button src="/images/but_del.gif" onClick="removeRow(document.getElementById(\'sources_table\'), \'' + newRow.id + '\')" >';
                newRow.appendChild(newCell);
                valueElem.value = "";
                valueElem.focus();
                return true;
            } else
                return false;
        }
        function removeSrcSubj(rowId)
        {
            var selectElem = document.getElementById('srcSubjSelect');
            var tbl = document.getElementById('sources_table');
            var rowElem = tbl.rows[rowId];
            var subjObj = findChildById(rowElem, 'subjSrc');
            var subjValue = subjObj.value;
            var oOption = document.createElement("OPTION");
            try {
              selectElem.add(oOption); // For MSIE
            } catch (ex) {
              selectElem.add(oOption, null);
            }
            oOption.innerText = oOption.text = oOption.value = subjValue;
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
    <hr>
    <table id=sources_table class=properties_list cellspacing=0 cellpadding=0>
        <col width="1%">
        <col width="100%">
        <%
            for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();) {
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
        }
        for (int i = 0; i < bean.getSrcMasks().length; i++) {
            String rowId = "maskRow_" + StringEncoderDecoder.encodeHEX(bean.getSrcMasks()[i]);
    %>
        <tr class=row<%=(rowN++) & 1%> id=<%=rowId%>>
            <td><img src="/images/mask.gif"></td>
            <td><%=bean.getSrcMasks()[i]%><input type=hidden name=srcMasks value="<%=bean.getSrcMasks()[i]%>"></td>
            <td><img class=button src="/images/but_del.gif"
                     onClick="removeRow(document.getElementById('sources_table'), '<%=rowId%>')">
            </td>
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
    if (validateField(document.getElementById('newDstMask'))) {
        var mask = document.getElementById('newDstMask').value;
        var smeSelect = document.getElementById('newDstMaskSme');
        var sme = smeSelect.options[smeSelect.selectedIndex].value;
        var tbl = document.getElementById('destinations_table');

        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "srcRow_" + (global_counter++);

        newCell = document.createElement("td");
        newCell.innerHTML = '<img src="/images/mask.gif">';
        newRow.appendChild(newCell);

        newCell = document.createElement("td");
        newCell.innerHTML = mask + '<input type=hidden name=dstMasks value="' + mask + '">';
        newRow.appendChild(newCell);

        newCell = document.createElement("td");
        newCell.innerHTML = smesSelectText;
        newSelect = findChildById(newCell, 'newSmesSelect');
        newSelect.name = "dst_mask_sme_" + encodeHEX(mask);
        var optObj = findChildById(newSelect, 'option_' + encodeHEX(sme));
        optObj.selected = true;
        newRow.appendChild(newCell);

        newCell = document.createElement("td");
        newCell.innerHTML = '<img class=button src="/images/but_del.gif" onClick="removeRow(document.getElementById(\'destinations_table\'), \'' + newRow.id + '\')">';
        newRow.appendChild(newCell);
        document.getElementById('newDstMask').value = "";
        document.getElementById('newDstMask').focus();
        return true;
    } else
        return false;
}
function selectDefaultSme() {
    var selectElem = document.getElementById('dstSubjSelect');
    var smeSelectElem = document.getElementById('dstSubjSmeSelect');
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
    var selectElem = document.getElementById('dstSubjSelect');
    var tbl = document.getElementById('destinations_table');
    var rowElem = tbl.rows[rowId];
    var subjObj = findChildById(rowElem, 'subjDst');
    var subjValue = subjObj.value;
    var subjDSObj = findChildById(rowElem, 'subjDst');
    var subjDefaultSme = subjDSObj.defaultSme;
    var oOption = document.createElement("OPTION");
    try {
      selectElem.add(oOption); // For MSIE
    } catch (ex) {
      selectElem.add(oOption, null); // For Firefox
    }
    oOption.innerText = oOption.text = oOption.value = subjValue;
    oOption.defaultSme = subjDefaultSme;
    selectElem.disabled = false;
    tbl.deleteRow(rowElem.rowIndex);
    return selectDefaultSme();
}
function addDestSubj() {
    var selectElem = document.getElementById('dstSubjSelect');
    if (selectElem.options.length > 0) {
        var subjValue = selectElem.options[selectElem.selectedIndex].value;
        var subjDefaultSme = selectElem.options[selectElem.selectedIndex].defaultSme;
        var tbl = document.getElementById('destinations_table');
        var smeSelect = document.getElementById('dstSubjSmeSelect');
        var sme = smeSelect.options[smeSelect.selectedIndex].value;
        var newRow = tbl.insertRow(tbl.rows.length);
        newRow.className = "row" + ((tbl.rows.length + 1) & 1);
        newRow.id = "srcRow_" + (global_counter++);

        newCell = document.createElement("td");
        newCell.innerHTML = '<img src="/images/subject.gif">';
        newRow.appendChild(newCell);

        newCell = document.createElement("td");
        newCell.innerHTML = subjValue + '<input id=subjDst type=hidden name=checkedDestinations value="' + subjValue + '">';
        var subjDstObj = findChildById(newCell, 'subjDst');
        subjDstObj.defaultSme = subjDefaultSme;
        newRow.appendChild(newCell);

        newCell = document.createElement("td");
        newCell.innerHTML = smesSelectText;
        newSelect = findChildById(newCell, 'newSmesSelect');
        newSelect.name = "dst_sme_" + encodeHEX(subjValue);
        var optObj = findChildById(newSelect, 'option_' + encodeHEX(sme));
        optObj.selected = true;
        newRow.appendChild(newCell);

        newCell = document.createElement("td");
        newCell.innerHTML = '<img class=button src="/images/but_del.gif" onClick="removeDestSubj(\'' + newRow.id + '\');">';
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
    <%rowN = 0;
        for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();) {
            String name = (String) i.next();
            if (bean.isDstChecked(name)) {
                String encName = StringEncoderDecoder.encode(name);
                String hexName = StringEncoderDecoder.encodeHEX(name);
                String rowId = "subjRow_" + StringEncoderDecoder.encodeHEX(name);
    %><tr class=row<%=(rowN++) & 1%> id=<%=rowId%>>
    <td><img src="/images/subject.gif"></td>
    <td><%=encName%><input id=subjDst type=hidden name=checkedDestinations value="<%=encName%>"
                           defaultSme="<%=StringEncoderDecoder.encode(bean.getDefaultSubjectSme(name))%>"></td>
    <td align=RIGHT><select name=dst_sme_<%=hexName%>>
        <%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext();) {
            String smeId = (String) j.next();
            String encSmeId = StringEncoderDecoder.encode(smeId);
        %><option value="<%=encSmeId%>" <%=bean.isSmeSelected(name, smeId) ? "selected" : ""%>><%=encSmeId%></option><%
        }
    %>
    </select>
    </td>
    <td><img class=button src="/images/but_del.gif" onClick="removeDestSubj('<%=rowId%>');"></td>
</tr><%
        }
    }
    for (int i = 0; i < bean.getDstMasks().length; i++) {
        String dstMask = bean.getDstMasks()[i];
        String encMask = StringEncoderDecoder.encode(dstMask);
        String hexMask = StringEncoderDecoder.encodeHEX(dstMask);
        String rowId = "maskRow_" + StringEncoderDecoder.encodeHEX(dstMask);
%>
    <tr class=row<%=(rowN++) & 1%> id="<%=rowId%>">
        <td><img src="/images/mask.gif"></td>
        <td><%=encMask%><input type=hidden name=dstMasks value="<%=encMask%>"></td>
        <td align=RIGHT><select name=dst_mask_sme_<%=hexMask%> onkeyup="resetValidation(this)">
            <%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext();) {
                String smeId = (String) j.next();
                String encSmeId = StringEncoderDecoder.encode(smeId);
            %><option
                value="<%=encSmeId%>" <%=bean.isMaskSmeSelected(dstMask, smeId) ? "selected" : ""%>><%=encSmeId%></option><%
            }
        %>
        </select>
        </td>
        <td><img class=button src="/images/but_del.gif"
                 onClick="removeRow(document.getElementById('destinations_table'), '<%=rowId%>')">
        </td>
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