<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder" %>
<input type=hidden id=isChanged name=isChanged value="false">
<div class=content>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Common values ~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>
    Distributions list
</div>
<table class=properties_list cellspacing=0>
    <thead>
        <col width="15%" align=right>
        <col width="85%">
    </thead>
    <tr class=row0>
        <th>
            name:
        </th>
        <td>
            <input class=txt name=name
                   value="<%= StringEncoderDecoder.encode(bean.getName()) %>" <%= isEditing ? "readonly" : "" %>>
        </td>
    </tr>
    <tr class=row1>
        <th>
            owner:
        </th>
        <td>
            <%
                if (bean.isCreate()) {
            %>
            <input class=radio type=radio name=system id=system value=true
                   onClick="checkSystemRadio();" <%= bean.isSystem() ? "checked" : "" %>>
            <label>
                &nbsp;
                system
            </label>
            <input class=radio type=radio name=system id=system value=false
                   onClick="checkSystemRadio();" <%= bean.isSystem() ? "" : "checked" %>>
            <label>
                &nbsp;
            </label>
            <input class=txt name=owner id=ownerAddress value="<%= StringEncoderDecoder.encode(bean.getOwner()) %>"
                       validation="address" onkeyup="resetValidation(this)">

            <%
            } else {
            %>
            <%= bean.isSystem()
                    ? "SYSTEM"
                    : StringEncoderDecoder.encode(bean.getOwner()) %><%
            }
        %>
        </td>
    </tr>
    <tr class=row0>
        <th>
            maximum members count:
        </th>
        <td>
            <input class=txt id="maxElements" name="maxElements"  value="<%= bean.getMaxElements() %>" validation="positive"
                   onkeyup="resetValidation(this)">
            <% if (bean.isCreate()) { %>
            <script>
                function checkSystemRadio() {
                    document.getElementById('ownerAddress').disabled = document.getElementById('system').checked;
                    document.getElementById('maxElements').disabled = !document.getElementById('system').checked;
                }

                checkSystemRadio();
            </script>
            <% } %>
        </td>
    </tr>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Submitters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>
    Submitters
</div>
<script>
    function removeSubmitters(rowId) {
        var t = document.getElementById('SubmittersTable');
        var r = t.rows[rowId];

        if (r != null) {
            t.deleteRow(r.rowIndex);
            document.getElementById('isChanged').value = "true";
        }
    }

    var rowCounter = 0;

    function addSubmitters() {
        var fme = document.getElementById('newSubmitters');
        var fm = fme.value;

        if (fm == null || fm == "") {
            return false;
        }

        var t = document.getElementById('SubmittersTable');
        var r = t.insertRow(t.rows.length - 1);

        r.id = "row_Submitters_" + rowCounter++;

        var c1 = r.insertCell(0);

        c1.innerHTML = "<input class=txt name=submitters value=\"" + fm +
                       "\" validation=\"address\" onkeyup=\"resetValidation(this)\">";

        var c2 = r.insertCell(1);

        c2.innerHTML = "<img class=button src=\"/images/but_del.gif\" onclick=\"removeSubmitters('" + r.id + "')\" >";
        fme.value = "";

        fme.focus();
        document.getElementById('isChanged').value = "true";
    }
</script>
<table id=SubmittersTable class=list cellspacing=0 cellpadding=1 width="100%">
    <thead>
        <col width="15%">
        <col width="84%">
    </thead>
    <%
        for (int i = 0; i < bean.getSubmitters().length; i++) {
            final String Submitters = bean.getSubmitters()[i];
            final String SubmittersHex = StringEncoderDecoder.encodeHEX(Submitters);
    %>
    <tr>
    <tr id=SubmittersRow_<%= SubmittersHex %> class=row0>
        <td colspan=1>
            <input class=txt name=submitters value="<%= StringEncoderDecoder.encode(bean.getSubmitters()[i]) %>"
                   validation="address"
                   onkeyup="resetValidation(this)" <%= isEditing && !bean.isSystem() && bean.getOwner().equals(bean.getSubmitters()[i]) ? "readonly" : "" %>>
        </td>
        <%-- <td><img src="/images/but_del.gif" onclick="removeSubmitters('SubmittersRow_<%=SubmittersHex%>')" style="cursor:hand;"></td>  --%>
        <td>
            <%
                out.print("<img class=button id=\"mbDel\" src=\"/images/but_del.gif\" title=\"delete submitter\"" +
                        "onClick = \"removeSubmitters('SubmittersRow_" + SubmittersHex + "');return false;\">");
            %>
        </td>
    </tr>
    <%
        }
    %>
    <tr class=row0>
        <td>
            <input class=txt name=submitters validation="address" id=newSubmitters onkeyup="resetValidation(this)">
        </td>
        <td>
            <%
                out.print("<img class=button id=\"mbAdd\" src=\"/images/but_add.gif\" title=\"add submitter\" " +
                        "onClick = \"addSubmitters();return false;\">");
            %>
        </td>
    </tr>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<input type=hidden name=startPosition value="<%= bean.getStartPosition() %>">
<input type=hidden name=totalSize value=<%= bean.getTotalSize() %>>
<input type=hidden name=sort>
<script>
    function setSort(sorting) {
        if (sorting == "<%= bean.getSort() %>") {
            opForm.sort.value = "-<%= bean.getSort() %>";
        } else {
            opForm.sort.value = sorting;
        }

        opForm.submit();

        return false;
    }
</script>

<div class=page_subtitle>
    Members
</div>
<script>
    function removeMembers(rowId) {
        var t = document.getElementById('MembersTable');
        var r = t.rows[rowId];

        if (r != null) {
            t.deleteRow(r.rowIndex);
            document.getElementById('isChanged').value = "true";
        }
    }

    var rowCounter = 0;

    function addMembers() {
        var fme = document.getElementById('newMembers');
        var fm = fme.value;

        if (fm == null || fm == "") {
            return false;
        }

        var t = document.getElementById('MembersTable');
        var r = t.insertRow(t.rows.length);

        r.id = "row_Members_" + rowCounter++;

        var c1 = r.insertCell(0);

        c1.innerHTML = "<input class=txt name=members value=\"" + fm +
                       "\" validation=\"address\" onkeyup=\"resetValidation(this)\">";

        var c2 = r.insertCell(1);

        c2.innerHTML = "<img class=button src=\"/images/but_del.gif\" onclick=\"removeMembers('" + r.id + "')\">";
        fme.value = "";

        fme.focus();
        document.getElementById('isChanged').value = "true";
    }
</script>
<table id=MembersTable class=list cellspacing=0 cellpadding=1 width="100%">
    <col width="15%">
    <col width="85%">
    <thead>
        <tr>
            <th class=secInner colspan=2 style="border-bottom: 1px solid #888888;">
                <a href="javascript:setSort('address')" <%= bean.getSort().endsWith("address") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : "" %>
                   title="address">Addresses</a>
            </th>
        </tr>
    </thead>
    <%
        for (int i = 0; i < bean.getMembers().length; i++) {
            final String Members = bean.getMembers()[i];
            final String MembersHex = StringEncoderDecoder.encodeHEX(Members);
    %>
    <tr id=MembersRow_<%= MembersHex %> class=row0>
        <td colspan=1>
            <input class=txt name=members value="<%= StringEncoderDecoder.encode(bean.getMembers()[i]) %>"
                   validation="address" onkeyup="resetValidation(this)">
        </td>
        <td>
            <%
                out.print("<img class=button id=\"mbDel\" src=\"/images/but_del.gif\" title=\"delete member\" " +
                        "onClick = \"removeMembers('MembersRow_" + MembersHex + "');return false;\">");
            %>
        </td>
    </tr>
    <%
        }
    %>
</table>
<%--<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>--%>
<table class=list cellspacing=0 cellpadding=1 width="100%">
    <thead>
        <col width="15%" align=right>
        <col width="85%">
    </thead>
    <tr class=row0>
        <td>
            <input class=txt name=members validation="address" id=newMembers onkeyup="resetValidation(this)">
        </td>
        <td>
            <%
                out.print("<img class=button id=\"mbAdd\" src=\"/images/but_add.gif\" title=\"add member\" " +
                        "onClick = \"addMembers();return false;\">");
            %>
        </td>
    </tr>
</table>
</div>
