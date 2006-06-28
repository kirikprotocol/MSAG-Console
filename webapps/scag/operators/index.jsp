<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="operators.index.title">
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbAdd" value="operators.index.item.mbadd.value" title="operators.index.item.mbadd.title"/>
            <sm-pm:item name="mbDelete" value="operators.index.item.mbdelete.value" title="operators.index.item.mbdelete.title"
                        onclick="return deleteConfirm()" isCheckSensible="true"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
        <sm:table columns="checkbox,name,description" names="c,operators.index.table.names.name,operators.index.table.names.description" widths="1,48,48" edit="name"/>
    </jsp:body>
</sm:page>