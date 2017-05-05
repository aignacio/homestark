var clock_second = 1000,
    clock_minute = 1000*60,
    clock_hour   = 1000*60*60;
var exports = module.exports = {
  /******************  Timers related to snmp_scan.js *************************/
  'timeReqGet':2*clock_second,
  'timeCheckList': 30*clock_second,
  'timeResendGet': 30*clock_hour,
  /******************  Timers related to coap_req.js **************************/
  'timeDBdevices': 5*clock_second,
  'timeScandevices': 5*clock_second
};
