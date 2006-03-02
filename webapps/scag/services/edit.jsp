<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page>
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">Create new Service Providers</c:when>
            <c:otherwise>Edit Service Provider</c:otherwise>
        </c:choose>
    </jsp:attribute>

    <jsp:attribute name="menu">
        <sm-pm:menu>
            <c:if test="${true}">
                <sm-pm:item name="mbAddChild" value="Add Service" title="Add new Service"/>
            </c:if>
            <c:if test="${!param.add}">
                <sm-pm:item name="mbDelete" value="Delete Service" title="Delete Service"
                            onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
            </c:if>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
        <sm-ep:properties title="Service Provider info">
            <c:if test="${!bean.add}"><input type="hidden" name="id" id="id" value="${fn:escapeXml(bean.id)}"></c:if>
            <sm-ep:txt title="name" name="name" validation="nonEmpty"/>
            <sm-ep:txtBox title="description" cols="0" rows="0" name="description"/>
        </sm-ep:properties>
        <div class=page_subtitle>&nbsp;</div>
        <sm-pm:menu>
            <c:if test="${!param.add}">
                <sm-pm:item name="mbSave" value="Done" title="Save provider info"/>
            </c:if>
            <c:if test="${param.add}">
                <sm-pm:item name="mbSave" value="Create" title="Create provider info"/>
            </c:if>
            <sm-pm:item name="mbCancel" value="Cancel" title="Cancel provider editing" onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
        <c:if test="${!param.add}">
            <div class=page_subtitle>&nbsp;</div>

            <div class=page_subtitle>Services List</div>
            <sm:table columns="checkbox,name" names="c,name,services num" widths="1,20" child="/service" edit="name"/>
        </c:if>
    </jsp:body>
</sm:page>