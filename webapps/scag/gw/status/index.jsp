<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Status" onLoad="enableDisableStartStopButtonsForSCAGStatusPage();">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbApply" value="Apply" title="Apply changes" isCheckSensible="true"/>
      <sm-pm:item name="mbRestore" value="Restore" title="Undo changes" isCheckSensible="true"/>
      <sm-pm:space/>
      <sm-pm:item name="mbStart" value="Start" title="Start SCAG" isCheckSensible="false"/>
      <sm-pm:item name="mbStop" value="Stop" title="Stop SCAG" isCheckSensible="false"/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <table class="list" cellspacing="0">
      <col width="1px"/>
      <tr class="row0">
        <td><input class="check" type="checkbox" name="subj" id="chk_cofig"     value="config" <c:if test="${!bean.configChanged}">disabled</c:if>></td>
        <td><label for="chk_cofig">GW config</label></td><td><sm:status changed="${bean.configChanged}"/></td>
      </tr>
      <tr class="row1">
        <td><input class="check" type="checkbox" name="subj" id="chk_routes"    value="routes" <c:if test="${!bean.routesChanged}">disabled</c:if>></td>
        <td><label for="chk_routes">Routes and subjects</label></td><td><sm:status changed="${bean.routesChanged}"/></td>
      </tr>
      <tr class="row0">
        <td><input class="check" type="checkbox" name="subj" id="chk_billing"    value="billing" <c:if test="${!bean.billingChanged}">disabled</c:if>></td>
        <td><label for="chk_billing">Route billing rules</label></td><td><sm:status changed="${bean.billingChanged}"/></td>
      </tr>
      <tr class="row1">
        <td><input class="check" type="checkbox" name="subj" id="chk_providers" value="providers" <c:if test="${!bean.providersChanged}">disabled</c:if>></td>
        <td><label for="chk_providers">Providers</label></td><td><sm:status changed="${bean.providersChanged}"/></td>
      </tr>
    <%--  <tr class="row0">
        <td><input class="check" type="checkbox" name="subj" id="chk_smscs"     value="smscs" <c:if test="${!bean.smscsChanged}">disabled</c:if>></td>
        <td><label for="chk_smscs">Service centers</label></td><td><sm:status changed="${bean.smscsChanged}"/></td>
      </tr>
    --%>
      <tr class="row0">
        <td><input class="check" type="checkbox" name="subj" id="chk_users"     value="users" <c:if test="${!bean.usersChanged}">disabled</c:if>></td>
        <td><label for="chk_users">Users</label></td><td><sm:status changed="${bean.usersChanged}"/></td>
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
    function enableDisableStartStopButtonsForSCAGStatusPage()
    {
      enableDisableByIdFunction('mbStart', document.all.SCAGStatusSpan.innerText != ' stopped');
      enableDisableByIdFunction('mbStop', document.all.SCAGStatusSpan.innerText != ' running');
      window.setTimeout(enableDisableStartStopButtonsForSCAGStatusPage, 1000);
    }
    </script>
  </jsp:body>
</sm:page>