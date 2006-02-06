<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page>
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">Create new user"</c:when>
            <c:otherwise>Edit user "${param.editId}"</c:otherwise>
        </c:choose>
    </jsp:attribute>

    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbSave" value="Save" title="Save user info"/>
            <sm-pm:item name="mbCancel" value="Cancel" title="Cancel user editing" onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
        <sm-ep:properties title="User info">
            <sm-ep:txt title="login" name="login" validation="nonEmpty" readonly="${!bean.add}"/>
            <c:choose>
                <c:when test="${bean.add}">
                    <sm-ep:txt title="password" name="password" validation="nonEmpty" type="password"/>
                    <sm-ep:txt title="confirm password" name="confirmPassword" validation="nonEmpty" type="password"/>
                </c:when>
                <c:otherwise>
                    <sm-ep:txt title="password" name="password" type="password"/>
                    <sm-ep:txt title="confirm password" name="confirmPassword" type="password"/>
                </c:otherwise>
            </c:choose>

            <sm-ep:list title="provider" name="providerId" values="${fn:join(bean.providerIds, ',')}"
                        valueTitles="${fn:join(bean.providerNames, ',')}" rowId="id_Providers_list"/>
            <sm-ep:checks title="roles" name="roles" values="gw,routing,services,stat"
                          valueTitles="Scag configuration,Routes configuration,Services,Statistics"/>
            <sm-ep:txt title="first name" name="firstName" validation="nonEmpty"/>
            <sm-ep:txt title="last name" name="lastName" validation="nonEmpty"/>
            <sm-ep:txt title="departament" name="dept"/>
            <sm-ep:txt title="work phone" name="workPhone" validation="nonEmpty"/>
            <sm-ep:txt title="home phone" name="homePhone"/>
            <sm-ep:txt title="cellular phone" name="cellPhone"/>
            <sm-ep:txt title="email" name="email" validation="email"/>
        </sm-ep:properties>
    </jsp:body>
</sm:page>