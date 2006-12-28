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

            <%--sm-ep:list title="provider" name="providerId" values="${fn:join(bean.providerIds, ',')}"
                        valueTitles="${fn:join(bean.providerNames, ',')}" rowId="id_Providers_list"/--%>

            <sm-ep:checks title="users.edit.checks.title" name="roles" values="gw,super_admin,management,routing,stat"
                          valueTitles="users.edit.checks.values.gw,users.edit.checks.values.super_admin,users.edit.checks.values.management,users.edit.checks.values.routing,users.edit.checks.values.stat"/>
            <sm-ep:property title="users.edit.property.preferences">
                  <sm-et:section title="Performance monitor" name="perfmon">
                      <sm-et:properties>
                          <sm-et:txt title="pixPerSecond" name="pixPerSecond" validation="positive"/>
                          <sm-et:txt title="scale" name="scale" validation="positive"/>
                          <sm-et:txt title="block" name="block" validation="positive"/>
                          <sm-et:txt title="vLightGrid" name="vLightGrid" validation="positive"/>
                          <sm-et:txt title="vMinuteGrid" name="vMinuteGrid" validation="positive"/>
                      </sm-et:properties>
                  </sm-et:section>
                  <sm-et:section title="Center monitor" name="scmon">
                      <sm-et:properties>
                          <sm-et:txt title="graph.scale" name="graph.scale" validation="positive"/>
                          <sm-et:txt title="graph.grid" name="graph.grid" validation="positive"/>
                          <sm-et:txt title="graph.higrid" name="graph.higrid" validation="positive"/>
                          <sm-et:txt title="graph.head" name="graph.head" validation="positive"/>
                          <sm-et:txt title="max.speed" name="max.speed" validation="positive"/>
                      </sm-et:properties>
                  </sm-et:section>
                  <sm-et:section title="Service points monitor" name="svcmon">
                      <sm-et:properties>
                          <sm-et:txt title="graph.scale" name="graph.scale" validation="positive"/>
                          <sm-et:txt title="graph.grid" name="graph.grid" validation="positive"/>
                          <sm-et:txt title="graph.higrid" name="graph.higrid" validation="positive"/>
                          <sm-et:txt title="graph.head" name="graph.head" validation="positive"/>
                          <sm-et:txt title="max.speed" name="max.speed" validation="positive"/>
                      </sm-et:properties>
                  </sm-et:section>
            </sm-ep:property>
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