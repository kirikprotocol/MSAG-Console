<%@ page import="java.util.Iterator,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%><div class=content>

    <script>
      var global_counter = 0;
    </script>
        <script>
        function addMask(valueElem) {
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
                newCell.innerHTML = '<img src="/images/but_del.gif" onClick="removeRow(document.getElementById(\'sources_table\'), \'' + newRow.id + '\')" style="cursor: hand;">';
                newRow.appendChild(newCell);
                valueElem.value = "";
                valueElem.focus();
                return true;
            } else
                return false;
        }
        function removeSubj(rowId)
        {
            var selectElem = document.getElementById('subjSelect');
            var tbl = document.getElementById('sources_table');
            var rowElem = tbl.rows(rowId);
            var subjObj = findChildById(rowElem, 'subjSrc');
            var subjValue = subjObj.value;
            var oOption = document.createElement("OPTION");
            selectElem.add(oOption);
            oOption.innerText = subjValue;
            oOption.value = subjValue;
            selectElem.disabled = false;
            tbl.deleteRow(rowElem.rowIndex);
        }
        function addSubj() {
            var selectElem = document.getElementById('subjSelect');
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
                newCell.innerHTML = '<img src="/images/but_del.gif" onClick="removeSubj(\'' + newRow.id + '\');" style="cursor: hand;">';
                newRow.appendChild(newCell);
                selectElem.options[selectElem.selectedIndex] = null;
                selectElem.focus();
                if (selectElem.options.length == 0)
                    selectElem.disabled = true;
            }
        }
    </script>
    <div class=page_subtitle><%=getLocString("subjects.subTitle")%></div>
    <%int rowN = 0;%>
    <%
        //if (beanResult == bean.RESULT_OK)
        {
    %>
    <table class=properties_list cellspacing=0 cellpadding=0>
        <col width="15%">
        <col width="85%">
        <tr class=row<%=(rowN++) & 1%>>
            <th><%=getLocString("subjects.name")%></th>
            <td><input class=txt name=name value="<%=bean.getName()%>" <%=isNew?"":"readonly"%>></td>
        </tr>
        <tr class=row<%=(rowN++) & 1%>>
            <th><%=getLocString("subjects.notes")%></th>
            <td><input class=txt name=notes value="<%=bean.getNotes()%>"></td>
        </tr>
        <tr class=row<%=(rowN++) & 1%>>
            <th><%=getLocString("subjects.defaultSme")%></th>
            <td><select name=defSme>
                <%for (Iterator i = bean.getPossibleSmes().iterator(); i.hasNext();) {
                    String smeId = (String) i.next();
                    String encSmeId = StringEncoderDecoder.encode(smeId);
                %><option
                    value="<%=encSmeId%>" <%=smeId.equals(bean.getDefSme()) ? "selected" : ""%>><%=encSmeId%></option><%
                }%>
            </select></td>
        </tr>
    </table>

    <div class=page_subtitle><%=getLocString("common.titles.masks.and.subjects")%></div>
    <table class=properties_list cellspacing=0 cellpadding=0>
<%--        <col width="15%">--%>
<%--        <col width="85%">--%>
<%--        <%--%>
<%--            for (int i = 0; i < bean.getMasks().length; i++) {--%>
<%--        %>--%>
<%--        <tr class=row<%=(rowN++) & 1%>>--%>
<%--            <td colspan=2><input class=txt name=masks value="<%=bean.getMasks()[i]%>" validation="mask"--%>
<%--                                 onkeyup="resetValidation(this)"></td>--%>
<%--        </tr>--%>
<%--        <%}%>--%>
<%--        <tr class=row<%=(rowN++) & 1%>>--%>
<%--            <td><input class=txt name=masks validation="mask" onkeyup="resetValidation(this)"></td>--%>
<%--            <td><%addButton(out, "mbAdd", "Add", "subjects.addMask2SubjectHint");%></td>--%>
<%--        </tr>--%>
    <col width="50%">
    <col width="50%">
    <tr class=row<%=(rowN++) & 1%>>
      <td valign=top><%rowN = 0;%>
        <table cellspacing=0 cellpadding=0>
            <col width="50%" align=left>
            <col width="50%" align=right>
            <col width="0%" align=left>
            <!--col width="50%" align=right>
           <col width="0%"-->
            <tr valign="middle">
                <td><%=getLocString("common.util.Subject")%></td>
                <td><select id=subjSelect name="fake_name" class="txt"><%
                    for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();) {
                        String name = (String) i.next();
                        if (!bean.isSrcChecked(name) && !bean.getName().equals(name)) {
                            String encName = StringEncoderDecoder.encode(name);
                %><option value="<%=encName%>"><%=encName%></option><%
                        }
                    }%></select></td>
                <td><img src="/images/but_add.gif" onclick="addSubj()" style="cursor:hand;"></td>
            </tr><tr>
            <td><%=getLocString("common.util.Mask")%></td>
            <td><input id=newSrcMask class=txt name=srcMasks validation="routeMask" onkeyup="resetValidation(this)">
            </td>
            <td><img src="/images/but_add.gif" onclick="addMask(document.getElementById('newSrcMask'))"
                     style="cursor:hand;"></td>
        </tr>
        </table>
    </td>
    <td>&nbsp;</td>
    </tr>
    <tr class=row<%=(rowN++) & 1%>>
    <td valign=top><%rowN = 0;%>
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
            <td><img src="/images/but_del.gif" onClick="removeSubj('<%=rowId%>');" style="cursor: hand;"></td>
        </tr><%
            }
        }
        for (int i = 0; i < bean.getSrcMasks().length; i++) {
            String rowId = "maskRow_" + StringEncoderDecoder.encodeHEX(bean.getSrcMasks()[i]);
    %>
        <tr class=row<%=(rowN++) & 1%> id=<%=rowId%>>
            <td><img src="/images/mask.gif"></td>
            <td><%=bean.getSrcMasks()[i]%><input type=hidden name=srcMasks value="<%=bean.getSrcMasks()[i]%>"></td>
            <td><img src="/images/but_del.gif"
                     onClick="removeRow(document.getElementById('sources_table'), '<%=rowId%>')" style="cursor: hand;">
            </td>
        </tr><%
        }%>
      </table>
    </td>
    <td>&nbsp;</td>
    </tr>
    </table>
    <%}%>

</div>