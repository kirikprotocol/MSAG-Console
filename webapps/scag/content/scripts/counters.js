var param_counter = 0;

function removeRow(tbl, rowId) {
    var tbl = getElementByIdUni(tbl);
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
function addRow(nameElem, valueElem, element, ffs, sfs){
    //console.info("nameElem="+nameElem);
    //console.info("valueElem="+valueElem);
    //console.info("element="+element);
    //console.info("ffs="+ffs);
    //console.info("sfs="+sfs);
    var nameElem = getElementByIdUni( nameElem );
    var valueElem = getElementByIdUni( valueElem );

    nameElem.style.color = "black";
    valueElem.style.color = "black";

    nameElem.style.borderColor = "black";
    valueElem.style.borderColor = "black";

    var paramValue = valueElem.value;
    //console.info(sfs+"="+paramValue);
    var paramName = nameElem.value;
    //console.info(ffs+"="+paramName);
    if (validateFirstField(element, nameElem)){
        if (trimStr(valueElem.value).length > 0) {
            if (unicName(paramName, element, ffs)) {
                //console.info("Parameter counter: "+param_counter);
                var inner__counter = param_counter++;
                var tbl = getElementByIdUni(element+".tbl");
                var newRow = tbl.insertRow(tbl.rows.length);
                newRow.className = "row" + ((tbl.rows.length + 1) & 1);
                    newRow.id = element + "."+ffs+"." + inner__counter;
                    //console.info("rowId="+element + "."+ffs+"." + inner__counter);
                    var newCell = document.createElement("td");
                    var sel_name = element+"."+sfs+"." + paramName + "." + inner__counter;
                    var input_name = element+"."+inner__counter+"."+ffs;
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
                    var sel_value = element+"."+sfs+"."+ paramName + "." + inner__counter;
                    input_name = element+"."+inner__counter+"."+sfs;
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
                            + "onClick=\"removeRow(\'"+element+".tbl\', " + "\'" + newRow.id + '\');\" style=\"cursor:pointer;\"/>';
                    newRow.appendChild(newCell);

                    nameElem.value = "";
                    valueElem.value = "";
                    nameElem.focus();
                    return true;
            } else {
                //console.info("Not unic name.");
                validationError(nameElem, "Not unic name.");
            }
        } else {
            //console.info("Field is empty.");
            validationError(valueElem, "Field is empty.");
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
        var IsFound = /^[a-zA-Z0-9\.]+$/.test(element.value);
        //console.info("isFound="+IsFound);
        if (!IsFound) {
            validationError(element, "Validation error! You can use only alphanumerical character and point to separate them.");
            return false;
        }
    }
    if (type == "limit"){
        validateInteger(element);

        var limitsMin = document.getElementsByName("limitsMin")[0];
        //console.info("limitsMin="+limitsMin.value);
        validateInteger(limitsMin);

        var limitsMax = document.getElementsByName("limitsMax")[0];
        //console.info("limitsMax="+limitsMax.value);
        validateInteger(limitsMax);

        //console.info("value="+element.value);
        if (element.value < limitsMin.value || element.value > limitsMax.value){
            validationError(element, "Value is limited by min and max values.");
            return false;
        }


    }
    return true;
}

function validateInteger(element){
    var IsFound = /^-?\d+$/.test(element.value);
    if (!IsFound) {
        validationError(element, "Validation error! Value must be decimal without sign.");
        return false;
    }
    return true;
}

