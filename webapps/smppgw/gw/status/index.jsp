<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Status" onLoad="enableDisableStartStopButtonsForSmppGwStatusPage();">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbApply" value="Apply" title="Apply changes" isCheckSensible="true"/>
      <sm-pm:item name="mbRestore" value="Restore" title="Undo changes" isCheckSensible="true"/>
      <sm-pm:space/>
      <sm-pm:item name="mbStart" value="Start" title="Start SMPP GW" isCheckSensible="false"/>
      <sm-pm:item name="mbStop" value="Stop" title="Stop SMPP GW" isCheckSensible="false"/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <table class="list" cellspacing="0">
      <col width="1px"/>
      <tr class="row0">
        <td><input class="check" type="checkbox" name="subj" id="chk_cofig" value="config" <c:if test="${!bean.configChanged}">disabled</c:if>></td><td><label for="chk_cofig">GW config</label></td><td><sm:status changed="${bean.configChanged}"/></td>
      </tr>
      <tr class="row1">
        <td><input class="check" type="checkbox" name="subj" id="chk_routes" value="routes" <c:if test="${!bean.routesChanged}">disabled</c:if>></td><td><label for="chk_routes">Routes</label></td><td><sm:status changed="${bean.routesChanged}"/></td>
      </tr>
      <tr class="row0">
        <td><input class="check" type="checkbox" name="subj" id="chk_users" value="users" <c:if test="${!bean.usersChanged}">disabled</c:if>></td><td><label for="chk_users">Users</label></td><td><sm:status changed="${bean.usersChanged}"/></td>
      </tr>
    </table>
    <script>
    function enableDisableByIdFunction(itemId, isDisabled)
    {
      var items = opForm.all[itemId];
      for (var i=0; i<items.length; i++) {
        items[i].disabled = isDisabled;
      }
    }
    function enableDisableStartStopButtonsForSmppGwStatusPage()
    {
      enableDisableByIdFunction('mbStart', document.all.SmppgwStatusSpan.innerText != ' stopped');
      enableDisableByIdFunction('mbStop', document.all.SmppgwStatusSpan.innerText != ' running');
      window.setTimeout(enableDisableStartStopButtonsForSmppGwStatusPage, 1000);
    }
    </script>
  </jsp:body>
</sm:page>