function resetValidation(elem)
{
	elem.errorMessage = null;
	elem.runtimeStyle.color = elem.style.color;
	if(elem.tagName == "SELECT")
	{
		elem = elem.parentElement;
	}
	elem.runtimeStyle.borderColor = elem.style.borderColor;
	var errPointer = elem.nextSibling;
	if(errPointer != null)
	{
		errPointer.style.display = "none";
	}
}
function validateShowErrors(elem)
{
	if(elem.errorMessage == null) return;
	elem.title = elem.errorMessage;
	/*
	var errPointer = elem.nextSibling;
	if((errPointer == null) || (errPointer.className != "errPointer"))
	{
		errPointer = document.createElement("DIV");
		errPointer = elem.parentElement.appendChild(errPointer);
		errPointer.className="errPointer";
	}
	errPointer.style.display = "block";
	errPointer.runtimeStyle.right = elem.runtimeStyle.posRight;
	errPointer.runtimeStyle.top = elem.runtimeStyle.posTop;*/
}

function validationError(elem, txt)
{
	elem.errorMessage = txt;

	elem.focus();

	elem.runtimeStyle.color = "#FF0000";
	if(elem.tagName == "SELECT")
	{
		elem = elem.parentElement;
	}
	elem.runtimeStyle.borderColor = "#FF0000";

	return false;
}

function validateField_port(elem)
{
	var intValue = elem.value/1;
	return isNaN(intValue) || intValue >= 0xFFFF
		? validationError(elem, "Port must be integer less than " + 0xFFFF)
		: true;
}

function validateField_mask(elem)
{
	if (elem.value == null || elem.value.length == 0)
	{
		return true;
	}
	var pattern_header = "^((\\.[0-6]\\.(0|1|3|4|6|8|9|10|14|18)\\.)|(\\+))?";
	var pattern1 = pattern_header + "\\d{1,20}\\?{0,19}$";
	var pattern2 = pattern_header + "(\\d|\\?){1,20}$";

	var r1 = RegExp(pattern1);
	var r2 = RegExp(pattern2);
	return elem.value == null || elem.value.match(r1) == null || elem.value.match(r2) == null
		? validationError(elem, "Invalid mask")
		: true;
}

function validateField_routeMask(elem)
{
	return validateField_mask(elem);
}

function validateField_select(elem)
{
	return (elem.selectedIndex == 0) 
		? validationError(elem, "Please, select something")
		: true;
}

function validateField_priority(elem)
{
	var intValue = elem.value/1;
	return isNaN(intValue) || intValue < 0  || intValue > (0x10000/2)
		? validationError(elem, "Priority must be positive integer less than " + (0x10000/2))
		: true;
}

function validateField_route_serviceId(elem)
{
	var intValue = elem.value/1;
	return isNaN(intValue) || intValue < 0
		? validationError(elem, "Service ID must be positive integer")
		: true;
}

function validateField_nonEmpty(elem)
{
	return elem.value == null || elem.value.length == 0
		? validationError(elem, "Must be not empty")
		: true;
}

function validateField_email(elem)
{
	var r = RegExp("(^).*\@.*\..*$");
	return elem.value == null || elem.value.match(r) == null
		? validationError(elem, "Invalid mask")
		: true;
}


function validateField(elem)
{
	switch(elem.validation)
	{
		case "port":return validateField_port(elem);
		case "mask":return validateField_mask(elem);
		case "routeMask":return validateField_routeMask(elem);
		case "select":return validateField_select(elem);
		case "priority":return validateField_priority(elem);
		case "route_serviceId": return validateField_route_serviceId(elem);
		case "nonEmpty": return validateField_nonEmpty(elem);
		case "email": return validateField_email(elem);
	}
	alert("unknown validation type:"+elem.validation);
	return false;
}

function validateForm(frm)
{
	var result = true;
	var ts = frm.elements;
	for (var i=ts.length-1; i>=0; i--)
	{
		var elem = opForm.elements(i);
		var validationClass = elem.validation;
		if(validationClass != null)
		{
			var r = validateField(elem);
			validateShowErrors(elem);
			result = result && r;
		}
	}
	return result;
}


function clickCancel()
{
	document.all.jbutton.name = "mbCancel";
	opForm.submit();
	return false;
}
