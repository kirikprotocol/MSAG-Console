var param_counter = 0;
var limit_counter = 0;

function removeRow(tblId, rowId) {
    var tbl = getElementByIdUni(tblId);
    for(var i=0; i<tbl.rows.length; i++){
        if(tbl.rows[i].id==rowId){
            tbl.deleteRow(i);
        }
    }
}

/*  ffs - first field suffix
 *  sfs - second field suffix
 *  tId - table identifier
  *  */
function addRow(id1, id2, type, ffs, sfs){
    //console.info("nameElem="+nameElem);
    //console.info("valueElem="+valueElem);
    //console.info("element="+element);
    //console.info("ffs="+ffs);
    //console.info("sfs="+sfs);
    var nameElem = getElementByIdUni( id1 );
    var valueElem = getElementByIdUni( id2 );

    nameElem.style.color = "black";
    valueElem.style.color = "black";

    nameElem.style.borderColor = "black";
    valueElem.style.borderColor = "black";

    var paramValue = valueElem.value.trim();
    //console.info(sfs+"="+paramValue);
    var paramName = nameElem.value.trim();

    if (type == "limit"){
        if (paramName.indexOf(">") == -1 && paramName.indexOf("<") == -1){
            paramName = ">" + paramName;
            //console.info("paramName="+paramName);
        }
    }
    //console.info(ffs+"="+paramName);
    if (validateFirstField(type, nameElem)){
        if (trimStr(valueElem.value).length > 0) {
            if (unicName(paramName, type, ffs)) {
                //console.info("Parameter counter: "+param_counter);
                var inner__counter = 0;
                //console.info("limit_counter="+limit_counter + " param_counter="+param_counter);
                if (type == "limit"){
                    inner__counter = limit_counter++;
                } else if (type == "parameter"){
                    inner__counter = param_counter++;
                }
                var tbl = getElementByIdUni(type+".tbl");
                var newRow = tbl.insertRow(tbl.rows.length);
                newRow.className = "row" + ((tbl.rows.length + 1) & 1);
                newRow.id = type + "."+ffs+"." + inner__counter;
                //console.info("rowId="+type + "."+ffs+"." + inner__counter);
                        var newCell = document.createElement("td");
                        var sel_name = type+"."+sfs+"." + paramName + "." + inner__counter;
                        var input_name = type+"."+inner__counter+"."+ffs;
                        //console.info("first input name="+input_name);
                        newCell.innerHTML = "<input id=\"" + sel_name + "\""
                                                    + " name=\""+input_name+"\""
                                                    + " type=\"text\""
                                                    + " size=\"45\""
                                                    + " readonly=\"true\""
                                                    + " value=\""+paramName+"\"/>";
                        newRow.appendChild(newCell);
                        newCell = document.createElement("td");
                        newCell.colSpan = 1;
                        var sel_value = type+"."+sfs+"."+ paramName + "." + inner__counter;
                        input_name = type+"."+inner__counter+"."+sfs;
                        //console.info("second input name="+input_name);
                        newCell.innerHTML = "<input id=\"" + sel_value + "\""
                                                    + " name=\""+input_name+"\""
                                                    + " type=\"text\" "
                                                    + " size=\"45\""
                                                    + " style=\"color:black;\""
                                                    + " value=\"" + paramValue + "\""
                                                    + " readonly=\"true\"/>";
                        newRow.appendChild(newCell);
                        newCell = document.createElement("td");
                        newCell.width = "100%";
                        newCell.innerHTML = "<img src=\"content/images/but_del.gif\" alt=\"Remove "
                                + paramName + " parameter\"  "
                                + "onClick=\"removeRow(\'"+type+".tbl\', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';
                        newRow.appendChild(newCell);

                        nameElem.value = "";
                        valueElem.value = "";
                        nameElem.focus();
                        return true;
                } else {
                    //console.info("Not unic name.");
                    validationError(nameElem, '<fmt:message>scripts.nameAlreadyUsed</fmt:message>');
                }
            } else {
                //console.info("Field is empty.");
                validationError(valueElem, '<fmt:message>scripts.nonEmptyErrorMsg</fmt:message>');
            }
        }

    return false;
}

function trimStr(sString) {
    while (sString.substring(0, 1) == ' ') {
        sString = sString.substring(1, sString.length);
    }
    while (sString.substring(sString.length - 1, sString.length) == ' ') {
        sString = sString.substring(0, sString.length - 1);
    }
    return sString;
}

function unicName(pName, type, ffs){
    //console.info("Check parameters name unicity.");
    //console.info("pName: "+pName);

    var inputs = document.getElementsByTagName("input");
    
    var l = inputs.length;
    //console.info("Parameters number: " + l);
    var input;
    for(var i=0; i<l; i++){
        input = inputs[i];
        //console.info("Parameter: " + parameter.name);
        if (input.name.indexOf(type+".") === 0){
            if (input.name.indexOf("."+ffs) > 0){
                //console.info("Parameter: " + parameter.value);
                if (input.value == pName) {
                    //console.info("This parameters name already is used.");
                    return false;
                } /*else {
                    console.info(parameter.value + " != " + pName);
                }   */
            }
        }
    }
    return true;
}

function validateFirstField(type, element){
    //console.info("validateFirstField() type="+type+" value="+element.value);
    if (type == "parameter"){
        var IsFound = /^[a-zA-Z]+[a-zA-z0-9\._]*$/.test(element.value);
        //console.info("isFound="+IsFound);
        if (!IsFound) {
            validationError(element, '<fmt:message>scripts.alphanumerical</fmt:message>');
            return false;
        }
    }
    if (type == "limit"){
        if (validateLimit(element)){

            var limitsMin = document.getElementsByName("limitsMin")[0];
            //console.info("limitsMin="+limitsMin.value);
            if (validateInteger(limitsMin)){

                var limitsMax = document.getElementsByName("limitsMax")[0];
                //console.info("limitsMax="+limitsMax.value);
                if (validateInteger(limitsMax)){

                    var value = element.value.trim();
                    //console.info("value="+value);
                    var limit;
                    if (value.indexOf(">") === 0 || value.indexOf("<") === 0){
                        //console.info("find characters more ore less");
                        limit = parseInt(value.substring(1, value.length));
                    } else{
                        limit = parseInt(value);
                    }
                    //console.info("limit="+limit);
                    //console.info("1:"+limit < parseInt(limitsMin.value));
                    //console.info("2:"+parseInt(limitsMax.value) < limit);
                    //console.info("1||2:"+limit < parseInt(limitsMin.value) || parseInt(limitsMax.value) < limit);
                    if (limit < 0 || 100 < limit){
                        validationError(element, '<fmt:message>scripts.percentErrorMsg</fmt:message>');
                        return false;
                    }
                } else{
                    return false;
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    //console.info("return true");
    return true;
}

function validateLimit(element){
    var IsFound = /^(>|<)?\d+$/.test(element.value);
    if (!IsFound) {
        validationError(element, '<fmt:message>scripts.valueSignErrorMsg</fmt:message>');
        return false;
    }
    return true;
}

function validateInteger(element){
    var IsFound = /^\d+$/.test(element.value);
    if (!IsFound) {
        validationError(element, '<fmt:message>scripts.valueSignErrorMsg</fmt:message>');
        return false;
    }
    return true;
}


