<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="Subjects">
    <jsp:attribute name="menu">
    </jsp:attribute>
    <jsp:body>

        <table class=properties_list cellspacing=0 cellpadding=0>
            <col width="15%">
            <col width="100%">
            <c:set var="prop_rowN" value="0" scope="request"/>
            <sm-ep:list title="Transport" onChange="changeTransportId();" name="transportId"
                        values="${fn:join(bean.transportIds, ',')}"
                        valueTitles="${fn:join(bean.transportTitles, ',')}"/>
        </table>
        <br>
        <c:choose>
            <c:when test="${bean.transportId == 1}">
                <div class="page_subtitle">SMPP Subject List</div>
                <sm:table columns="checkbox,name,notes" names="c,name,notes" widths="1,49,50" edit="name"/>
                <sm-pm:menu>
                    <sm-pm:item name="mbAdd" value="Add" title="Add new subject"/>
                    <sm-pm:item name="mbDelete" value="Delete" title="Delete subject"
                                onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
                    <sm-pm:space/>
                </sm-pm:menu>

            </c:when>
            <c:when test="${bean.transportId == 2}">
                <div class="page_subtitle">HTTP Abonents Subject List</div>
                <sm:subject columns="checkbox,name" names="c,name" widths="1,99" edit="name"/>
                <sm-pm:menu>
                    <sm-pm:item name="mbAddHttpSubj" value="Add" title="Add new HTTP Subject"/>
                    <sm-pm:item name="mbDeleteHttpSubj" value="Delete" title="Delete HTTP Subject"
                                onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
                    <sm-pm:space/>
                </sm-pm:menu>
                <div class=page_subtitle>&nbsp;</div>

                <div class="page_subtitle">HTTP Subjects Site List</div>
                <sm:site columns="checkbox,name" names="c,name" widths="1,99" edit="name"/>
                <sm-pm:menu>
                    <sm-pm:item name="mbAddHttpSite" value="Add" title="Add new subject"/>
                    <sm-pm:item name="mbDeleteHttpSite" value="Delete" title="Delete subject"
                                onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
                    <sm-pm:space/>
                </sm-pm:menu>

            </c:when>
            <c:when test="${bean.transportId == 3}">
            </c:when>
        </c:choose>
    </jsp:body>
</sm:page>