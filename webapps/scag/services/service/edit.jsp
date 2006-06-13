<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page>
    <jsp:attribute name="title">
        <c:choose>
            <c:when test="${param.add}">service.add.title</c:when>
            <c:otherwise>service.edit.title</c:otherwise>
        </c:choose>
    </jsp:attribute>

    <jsp:attribute name="menu">

    </jsp:attribute>

    <jsp:body>
        <sm-ep:properties title="service.edit.properties.service_info">
            <c:if test="${!bean.add}"><input type="hidden" name="id" id="id" value="${fn:escapeXml(bean.id)}"></c:if>
            <sm-ep:txt title="service.edit.txt.name" name="name" validation="nonEmpty"/>
            <sm-ep:txtBox title="service.edit.txtbox.description" cols="0" rows="0" name="description"/>
            <input type="hidden" id="parentId" name="parentId" value="${fn:escapeXml(bean.parentId)}">
        </sm-ep:properties>

        <div class=page_subtitle>&nbsp;</div>
        <sm-pm:menu>
            <c:if test="${!param.add}">
                <sm-pm:item name="mbSave" value="service.edit.item.mbsave.value" title="service.edit.item.mbsave.title"/>
            </c:if>
            <c:if test="${param.add}">
                <sm-pm:item name="mbSave" value="service.edit.item.mbsave2.value" title="service.edit.item.mbsave2.title"/>
            </c:if>
            <sm-pm:item name="mbCancel" value="service.edit.item.mbcancel.value" title="service.edit.item.mbcancel.title" onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
        <c:if test="${!param.add}">
            <div class=page_subtitle>&nbsp;</div>
            <sm-ep:properties title="service.edit.properties.service_rules">
                <sm:rule transport="service.edit.rule.transport.smpp" exists="${bean.smppRuleExists}"/>
                <sm:rule transport="service.edit.rule.transport.http" exists="${bean.httpRuleExists}"/>
                <sm:rule transport="service.edit.rule.transport.mms" exists="${bean.mmsRuleExists}"/>
            </sm-ep:properties>
            <div class=page_subtitle>&nbsp;</div>
            <div class=page_subtitle><fmt:message>service.edit.label.routes_list</fmt:message></div>
            <sm:table columns="checkbox,id,active,enabled,archived,notes"
                      names="c,service.edit.table.names.id,service.edit.table.names.active,service.edit.table.names.enabled,service.edit.table.names.archived,service.edit.table.names.notes" widths="1,60,20,20,39,59"
                      child="/routing/routes" parentId="${bean.parentId}" edit="id"/>
            <div class=page_subtitle>&nbsp;</div>
            <sm-pm:menu>
                <sm-pm:item name="mbAddChild" value="service.edit.item.mbaddchild.value" title="service.edit.item.mbaddchild.title"/>
                <sm-pm:item name="mbDelete" value="service.edit.item.mbdelete.value" title="service.edit.item.mbdelete.title"
                            onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
                <sm-pm:space/>
            </sm-pm:menu>
        </c:if>
    </jsp:body>
</sm:page>