<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder" %>
<div class=content>
  <%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Common values ~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
  <div class=page_subtitle>
    <%= getLocString("dl.subTitle") %>
  </div>
  <table class=properties_list cellspacing=0>
    <thead>
      <col width="15%" align=right>
      <col width="85%">
    </thead>
    <tr class=row0>
      <th>
        <%= getLocString("dl.name") %>:
      </th>
      <td>
        <input class=txt name=name value="<%= StringEncoderDecoder.encode(bean.getName()) %>" <%= isEditing ? "readonly" : "" %>>
      </td>
    </tr>
    <tr class=row1>
      <th>
        <%= getLocString("dl.owner") %>:
      </th>
      <td>
<%
        if (bean.isCreate()) {
%>
          <input class=radio type=radio name=system id=systemTrue value=true onClick="checkSystemRadio();" <%= bean.isSystem() ? "checked" : "" %>>
          <label for=systemTrue>
            &nbsp;
            <%= getLocString("dl.owner.system") %>
          </label>
          <input class=radio type=radio name=system id=systemFalse value=false onClick="checkSystemRadio();" <%= bean.isSystem() ? "" : "checked" %>>
          <label for=systemFalse>
            &nbsp;
            <input class=txt name=owner id=ownerAddress value="<%= StringEncoderDecoder.encode(bean.getOwner()) %>" validation="address" onkeyup="resetValidation(this)">
          </label>
          <script>
            function checkSystemRadio() {
              document.getElementById('ownerAddress').disabled = document.getElementById('systemTrue').checked;
            }

            checkSystemRadio();
          </script>
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
        <%= getLocString("dl.maxMembersCount") %>:
      </th>
      <td>
        <input class=txt name=maxElements value="<%= bean.getMaxElements() %>" validation="positive" onkeyup="resetValidation(this)">
      </td>
    </tr>
  </table>
  <%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Submitters ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
  <div class=page_subtitle>
    <%= getLocString("dl.submittersTitle") %>
  </div>
  <script>
    function removeSubmitters(rowId) {
      var t = document.getElementById('SubmittersTable');
      var r = t.rows(rowId);

      if (r != null) {
        t.deleteRow(r.rowIndex);
      }
    }

    var rowCounter = 0;

    function addSubmitters() {
      var fme = document.getElementById('newSubmitters');
      var fm  = fme.value;

      if (fm == null || fm == "") {
        return false;
      }

      var t = document.getElementById('SubmittersTable');
      var r = t.insertRow(t.rows.length - 1);

      r.id = "row_Submitters_" + rowCounter++;

      var c1 = r.insertCell();

      c1.innerHTML = "<input class=txt name=submitters value=\"" + fm +
              "\" validation=\"address\" onkeyup=\"resetValidation(this)\">";

      var c2 = r.insertCell();

      c2.innerHTML = "<img src=\"/images/but_del.gif\" onclick=\"removeSubmitters('" + r.id + "')\" style=\"cursor:hand;\">";
      fme.value = "";

      fme.focus();
    }
  </script>
  <table id=SubmittersTable class=list cellspacing=0 cellpadding=1 width="100%">
    <thead>
      <col width="15%">
      <col width="84%">
    </thead>
<%
    for (int i = 0; i < bean.getSubmitters().length; i++) {
      final String Submitters    = bean.getSubmitters()[i];
      final String SubmittersHex = StringEncoderDecoder.encodeHEX(Submitters);
%>
      <tr>
        <tr id=SubmittersRow_<%= SubmittersHex %> class=row0>
          <td colspan=1>
            <input class=txt name=submitters value="<%= StringEncoderDecoder.encode(bean.getSubmitters()[i]) %>" validation="address" onkeyup="resetValidation(this)" <%= isEditing && !bean.isSystem() && bean.getOwner().equals(bean.getSubmitters()[i]) ? "readonly" : "" %>>
          </td>
          <%-- <td><img src="/images/but_del.gif" onclick="removeSubmitters('SubmittersRow_<%=SubmittersHex%>')" style="cursor:hand;"></td>  --%>
          <td>
<%
            delButton(out, "mbDel", "Del", "dl.delSubmitterHint", "removeSubmitters('SubmittersRow_" + SubmittersHex + "')");
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
          addButton(out, "mbAdd", "Add", "dl.addSubmitterHint", "addSubmitters()");
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
      <%= getLocString("dl.membersTitle") %>
    </div>
    <script>
      function removeMembers(rowId) {
        var t = document.getElementById('MembersTable');
        var r = t.rows(rowId);

        if (r != null) {
          t.deleteRow(r.rowIndex);
        }
      }

      var rowCounter = 0;

      function addMembers() {
        var fme = document.getElementById('newMembers');
        var fm  = fme.value;

        if (fm == null || fm == "") {
          return false;
        }

        var t = document.getElementById('MembersTable');
        var r = t.insertRow(t.rows.length - 1);

        r.id = "row_Members_" + rowCounter++;

        var c1 = r.insertCell();

        c1.innerHTML = "<input class=txt name=members value=\"" + fm +
                "\" validation=\"address\" onkeyup=\"resetValidation(this)\">";

        var c2 = r.insertCell();

        c2.innerHTML = "<img src=\"/images/but_del.gif\" onclick=\"removeMembers('" + r.id + "')\" style=\"cursor:hand;\">";
        fme.value = "";

        fme.focus();
      }
    </script>
    <table id=MembersTable class=list cellspacing=0 cellpadding=1 width="100%">
      <col width="15%">
      <col width="85%">
      <thead>
        <tr>
          <th class=secInner colspan=2 style="border-bottom: 1px solid #888888;">
            <a href="javascript:setSort('address')" <%= bean.getSort().endsWith("address") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : "" %> title="<%= getLocString("common.sortmodes.addressHint") %>"><%= getLocString("common.sortmodes.address") %></a>
          </th>
        </tr>
      </thead>
<%
      for (int i = 0; i < bean.getMembers().length; i++) {
        final String Members    = bean.getMembers()[i];
        final String MembersHex = StringEncoderDecoder.encodeHEX(Members);
%>
        <tr id=MembersRow_<%= MembersHex %> class=row0>
          <td colspan=1>
            <input class=txt name=members value="<%= StringEncoderDecoder.encode(bean.getMembers()[i]) %>" validation="address" onkeyup="resetValidation(this)">
          </td>
          <td>
<%
            delButton(out, "mbDel", "Del", "dl.delMemberHint", "removeMembers('MembersRow_" + MembersHex + "');return false;"
                    );
%>
          </td>
        </tr>
<%
      }
%>
    </table>
    <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>
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
          addButton(out, "mbAdd", "Add", "dl.addMemberHint", "addMembers();return false;");
%>
        </td>
      </tr>
    </table>
  </div>
