<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder" %>
<div class=content>
    <div class=page_subtitle><%=getLocString("closedgroups.subTitle")%></div>
    <script>
        function removeFilterMask(rowId) {
            var t = document.getElementById('filterMasksTable');
            var r = t.rows(rowId);

            if (r != null) {
                t.deleteRow(r.rowIndex);
            }
        }

        var rowCounter = 0;

        function addFilterMask() {
            var fme = document.getElementById('newFilterMask');
            var fm = fme.value;

            if (fm == null || fm == "") {
                return false;
            }

            var t = document.getElementById('filterMasksTable');
            var r = t.insertRow(t.rows.length - 1);

            r.id = "row_FilterMask_" + rowCounter++;

            var c1 = r.insertCell();

            c1.innerHTML = "<input class=txt name=masks value=\"" + fm +
                           "\" validation=\"mask\" onkeyup=\"resetValidation(this)\">";

            var c2 = r.insertCell();

            c2.innerHTML = "<img src=\"/images/but_del.gif\" onclick=\"removeFilterMask('" + r.id +
                           "')\" style=\"cursor:hand;\">";
            fme.value = "";

            fme.focus();
        }
    </script>

    <table class=properties_list cellspacing=0 cellspadding=0>
        <col width="15%">
        <col width="85%">
        <%int rowN = 0;%>
        <tr class=row<%=(rowN++) & 1%>>
            <th><%=getLocString("common.util.Name")%>:</th>
            <td><input class=txt name=name value="<%= StringEncoderDecoder.encode(bean.getName())%>"></td>
        </tr>
        <tr class=row<%=(rowN++) & 1%>>
            <th><%=getLocString("common.util.descr")%>:</th>
            <td><textarea rows="3" cols="20" name="def"
                          wrap="off"><%=bean.getDef() != null ? StringEncoderDecoder.encode(bean.getDef()) : ""%></textarea>
            </td>
        </tr>
        <tr class=row<%=(rowN++) & 1%>>
            <th><%= getLocString("common.util.addresses") %></th>
            <td>
                <table id=filterMasksTable>
                    <col width="1%">
                    <col width="1%">
                    <col width="98%">
                    <%
                        for (int i = 0; i < bean.getMasks().length; i++) {
                            final String filterMask = bean.getMasks()[i];
                            final String filterMaskHex = StringEncoderDecoder.encodeHEX(filterMask);
                    %>
                    <tr id=filterMaskRow_<%= filterMaskHex %>>
                        <td>
                            <input class=txt name=masks value="<%= StringEncoderDecoder.encode(filterMask) %>"
                                   validation="mask" onkeyup="resetValidation(this)">
                        </td>
                        <td><img src="/images/but_del.gif"
                                 onclick="removeFilterMask('filterMaskRow_<%= filterMaskHex %>')"
                                 style="cursor:hand;"></td>
                        <td>
                            &nbsp;
                        </td>
                    </tr>
                    <%
                        }
                    %>
                    <tr>
                        <td>
                            <input class=txt name=masks value="" id=newFilterMask validation="mask"
                                   onkeyup="resetValidation(this)">
                        </td>
                        <td><img src="/images/but_add.gif" onclick="addFilterMask()" style="cursor:hand;"></td>
                        <td>
                            &nbsp;
                        </td>
                    </tr>
                </table>
            </td>
        </tr>
    </table>
</div>