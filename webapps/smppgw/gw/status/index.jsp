<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Status">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbApply" value="Apply" title="Apply changes" isCheckSensible="true"/>
      <sm-pm:item name="mbRestore" value="Restore" title="Undo changes" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <table class="list" cellspacing="0">
      <col width="1px"/>
      <tr class="row0">
        <td><input class="check" type="checkbox" name="subj" value="config"></td><td>GW config</td>
      </tr>
    </table>
  </jsp:body>
</sm:page>