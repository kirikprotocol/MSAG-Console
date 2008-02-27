<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page>


    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">users.edit.title.create_user</c:when>
            <c:otherwise>users.edit.title.edit_user</c:otherwise>
        </c:choose>
    </jsp:attribute>

    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbSave" value="users.edit.item.mbsave.value" title="users.edit.item.mbsave.title"/>
            <sm-pm:item name="mbCancel" value="users.edit.item.mbcancel.value" title="users.edit.item.mbcancel.title" onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
    <script>
function collapsing_tree_click_tree_section(id)
{
  var bodyElem = document.getElementById(id);
 alert("ID='" + id + "'");
  if (bodyElem.style.display != 'none') {
    bodyElem.style.display = 'none';
    this.className = "closed";
  } else {
    bodyElem.style.display = '';
    this.className = "opened";
  }
  return false;
}

function closeProp(id)
{
//  alert("id=" + id);
    var tbl = getElementByIdUni(id);
    var divCat = getElementByIdUni(id+"_div");
//  alert( tbl+"_div" + " DIV=" + divCat );
    if(tbl.style.display=='none'){
        tbl.style.display = "block";
        divCat.className = "collapsing_tree_opened";
    } else {
        tbl.style.display = "none";
        divCat.className = "collapsing_tree_closed";
    }
}

</script>
        <sm-ep:properties title="users.edit.properties">
            <sm-ep:txt title="users.edit.txt.login" name="login" validation="nonEmpty" readonly="${!bean.add}"/>
            <c:choose>
                <c:when test="${bean.add}">
                    <sm-ep:txt title="users.edit.txt.password" name="password" validation="nonEmpty" type="password"/>
                    <sm-ep:txt title="users.edit.txt.confirmpassword" name="confirmPassword" validation="nonEmpty" type="password"/>
                </c:when>
                <c:otherwise>
                    <sm-ep:txt title="users.edit.txt.password" name="password" type="password"/>
                    <sm-ep:txt title="users.edit.txt.confirmpassword" name="confirmPassword" type="password"/>
                </c:otherwise>
            </c:choose>

            <sm-ep:checks title="users.edit.checks.title" name="roles" values="gw,super_admin,management,routing,stat"
                          valueTitles="users.edit.checks.values.gw,users.edit.checks.values.super_admin,users.edit.checks.values.management,users.edit.checks.values.routing,users.edit.checks.values.stat"/>
            <c:set var="prop_rowN_prev" value="${prop_rowN}" scope="request"/>
            <sm-ep:property title="users.edit.property.preferences">
                <sm-ep:properties>
                  <tr>
                    <td>
                        <sm-et:section title="Performance monitor" name="perfmon" type="table" classs="row${prop_rowN_prev%2}">
                            <sm-ep:properties id="perfmon" display="none">
                                <tr><td>
                                  <sm-et:section title="SMPP" name="smpp" type="table">
                                    <sm-ep:properties id="smpp" display="none">
                                          <sm-et:txt title="pixPerSecond" name="pixPerSecond" validation="positive"/>
                                          <sm-et:txt title="scale" name="scale" validation="positive"/>
                                          <sm-et:txt title="block" name="block" validation="positive"/>
                                    </sm-ep:properties>
                                  </sm-et:section>
                                  <sm-et:section title="HTTP" name="http" type="table">
                                      <sm-ep:properties id="http" display="none">
                                          <sm-et:txt title="pixPerSecond" name="pixPerSecond" validation="positive"/>
                                          <sm-et:txt title="scale" name="scale" validation="positive"/>
                                          <sm-et:txt title="block" name="block" validation="positive"/>
                                      </sm-ep:properties>
                                  </sm-et:section>
                                  <table class="list">
                                  <sm-et:txt title="vLightGrid" name="vLightGrid" validation="positive"/>
                                  <sm-et:txt title="vMinuteGrid" name="vMinuteGrid" validation="positive"/>
                                  </table>
                                </td></tr>
                              </sm-ep:properties>
                        </sm-et:section>

                 <sm-et:section title="Center monitor" name="scmon" type="table" classs="row${prop_rowN_prev%2}">
                        <sm-ep:properties id="scmon" display="none">
                              <sm-et:txt title="graph.scale" name="graph.scale" validation="positive"/>
                              <sm-et:txt title="graph.grid" name="graph.grid" validation="positive"/>
                              <sm-et:txt title="graph.higrid" name="graph.higrid" validation="positive"/>
                              <sm-et:txt title="graph.head" name="graph.head" validation="positive"/>
                              <sm-et:txt title="max.speed" name="max.speed" validation="positive"/>
                        </sm-ep:properties>
                  </sm-et:section>
                  <sm-et:section title="Service points monitor" name="svcmon" type="table" classs="row${prop_rowN_prev%2}">
                        <sm-ep:properties id="svcmon" display="none">
                          <sm-et:txt title="graph.scale" name="graph.scale" validation="positive" readonly="true"/>
                          <sm-et:txt title="graph.grid" name="graph.grid" validation="positive"/>
                          <sm-et:txt title="graph.higrid" name="graph.higrid" validation="positive"/>
                          <sm-et:txt title="graph.head" name="graph.head" validation="positive"/>
                          <sm-et:txt title="max.speed" name="max.speed" validation="positive"/>
                        </sm-ep:properties>
                  </sm-et:section>
                </td></tr>
              </sm-ep:properties>
            </sm-ep:property>
            <c:set var="prop_rowN" value="${prop_rowN_prev+1}" scope="request"/>
            <sm-ep:txt title="users.edit.txt.firstname" name="firstName" validation="nonEmpty"/>
            <sm-ep:txt title="users.edit.txt.lastmame" name="lastName" validation="nonEmpty"/>
            <sm-ep:txt title="users.edit.txt.dept" name="dept"/>
            <sm-ep:txt title="users.edit.txt.workphone" name="workPhone" validation="nonEmpty"/>
            <sm-ep:txt title="users.edit.txt.homephone" name="homePhone"/>
            <sm-ep:txt title="users.edit.txt.cellphone" name="cellPhone"/>
            <sm-ep:txt title="users.edit.txt.email" name="email" validation="email"/>
        </sm-ep:properties>
    </jsp:body>
</sm:page>