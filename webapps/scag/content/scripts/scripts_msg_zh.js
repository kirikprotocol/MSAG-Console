var portErrorMsg = "Port must be integer more than -1 and less than ";   //portErrorMsg+0xFFFF
var maskErrorMsg = "Invalid mask";
var idErrorMsg   = "Invalid id";
var addressErrorMsg = "Invalid address";
var addrPreErrorMsg = "Invalid address prefix";
var selectErrorMsg = "Please, select something";
var nonEmptyErrorMsg = "Must be not empty";
var valueErrorMsg = "Value must be a positive integer";
var valueSignErrorMsg = "Value must be decimal without sign";
var policyErrorMsg = "Invalid reschedule policy";
var nonLanguageErrorMsg = "Language is incorrect. Set to default.";

var priorityErrorMsg = "Priority must be positive integer less than "; // priorityErrorMsg + (0x10000/2)
var servIDErrorMsg = "Service ID must be positive integer";

var rangeValueErrorMsg_pre = "value must be an integer in range ["; // rangeValueErrorMsg_pre+elem.range_min+", "+elem.range_max+"]"

var causePositiveErrorMsg = "Release cause value must be a positive integer";
var causeNullValueErrorMsg = "Invalid release cause: value is null";
var causeValueErrorMsg_pre = "Invalid release cause: value '"; // causeValueErrorMsg_pre+intValue+causeValueErrorMsg_post
var causeValueErrorMsg_post = "' is undefined";

var unknownValidationTypeErrorMsg = "unknown validation type";
var nameAlreadyUsed="This name is already used";
var alphanumerical="You can use only alphanumerical character and point to separate them.";
var percentErrorMsg="Value is limited by '0' and '100' values.";