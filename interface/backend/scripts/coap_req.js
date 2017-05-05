/********************************* DISCLAIMER *********************************/

 // Comentado as linhas abaixo no arquivo node_modules/coap/lib/retry_send.js da
 // biblioteca do CoAP para node.js para evitar erro de timeout de reenvio.
 //
 //  67 // if (!avoidBackoff)
 //  68 //   this._timer = setTimeout(function() {
 //  69 //     var err  = new Error('No reply in ' + parameters.exchangeLifetime + 's')
 //  70 //     err.retransmitTimeout = parameters.exchangeLifetime;
 //  71 //     that.emit('error', err)
 //  72 //     that.emit('timeout', err)
 //  73 //

/********************************* DISCLAIMER *********************************/

var node = require('../models/nodes');
var dash_dev = require('../models/dash');
var constants = require('../var_constants');
var sha256 = require('sha256');
var coap = require('coap');
var colors = require('colors');
// var req = coap.request('coap://[aaaa::1]/test/hello').end();
var exports = module.exports = {};
var devices = [];
const util = require('util');
var global_devices_index = 0;

// var coapTiming = {
//   ackTimeout:0.25,
//   ackRandomFactor: 1.0,
//   maxRetransmit: 3,
//   maxLatency: 2,
//   piggybackReplyMs: 10
// };
// coap.updateTiming(coapTiming);

// var req  = coap.request('coap://[aaaa::212:4b00:802:b285]/test/hello').end();
// var req2 = coap.request('coap://[aaaa::212:4b00:802:b404]/test/hello').end();
// var req3  = coap.request('coap://[aaaa::212:4b00:802:b504]/test/hello').end();

// function callDevices(){
  // setTimeout(callDevices, 5000);
  // req = coap.request({
  //   host:'aaaa::212:4b00:802:b504',
  //   pathname:'/switch',
  //   method:'POST',
  //   confirmable:false});
  // req.write('switch_value=255');
  // req.setOption('Content-Format', 'text/plain');
  // req.end();

  // var devices = snmp.get_devices();
  // req  = coap.request('coap://[aaaa::212:4b00:802:b285]/test?anderson=teste').end();
  // req  = coap.request('coap://[aaaa::212:4b00:802:b404]/test?anderson=teste').end();

  // req2 = coap.request('coap://[aaaa::212:4b00:802:b404]/test/hello').end();
  // req3  = coap.request('coap://[aaaa::212:4b00:802:b504]/test/hello').end();
  // console.log("Dados:"+devices);
  // req2.on('response', function(res) {
  //   // console.log(res)
  //   res.pipe(process.stdout)
  //   res.on('end', function() {
  //     process.exit(0)
  //   })
  // })
  //
  // req3.on('response', function(res) {
  //   // console.log(res)
  //   res.pipe(process.stdout)
  //   res.on('end', function() {
  //     process.exit(0)
  //   })
  // })
// }
function updateDash(device){ // Atualiza ou cria sensor de dash com base na resposta GET, no modelo [tipo_de_sensor=valor&tipo_de_sensor_2=valor....]
  // console.log("HARDWARE ADDRES:"+device.hw_address);
  var types = device.value.split('&');
  // for (var i = 0; i < types.length; i++) {
    var typ_el = types[1].split('=');
    // console.log("A salvar no banco: -> "+typ_el[1]);
    dash_dev.findOne({'hw_assoc':device.hw_address.toLowerCase(),
                      'type':typ_el[0].toLowerCase()},
      function(err,dash){
        if (dash) {
          // console.log('Update nova dash: ['+device.hw_address+']['+typ_el[0]+']');
          dash_dev.update({ $and : [
                          {'hw_assoc':device.hw_address.toLowerCase()},
                          {'type':typ_el[0].toLowerCase()}]},
          { 'active'      : device.active,
            'type'        : typ_el[0].toLowerCase(),
            'value'       : typ_el[1],
            'hw_assoc'    : device.hw_address.toLowerCase(),
            'updated'     : new Date().toISOString()
          },function(err,node_data){
            if (err) {
              console.log('[Erro] ao atualizar dash no banco de dados: '+err);
            }
            // else {
            //  console.log('[MongoDB] Sucesso ao atualizar nó no banco de dados: [IPv6 Device]['+device.hw_address.toLowerCase()+']');
            // }
          });
        }
        else {
          var dash_sensor = new dash_dev();
          dash_sensor.value    = typ_el[1];
          dash_sensor.active   = device.active;
          dash_sensor.type     = typ_el[0];
          dash_sensor.hw_assoc = device.hw_address;
          dash_sensor.name     = "Sensor";

          // console.log('Cadastrando nova dash: ['+dash_sensor.hw_assoc+']['+dash_sensor.type+']');
          dash_sensor.save(function(err) {
              if (err){
                console.log('[Erro] ao salvar dash NOVA no banco de dados: '+err);
                throw err;
              }
          });
        }
    });
  // }
}

function dataUpdateDB(device){
  node.findOne({'hw_address':device.hw_address.toLowerCase()}, function(err,node_data){
    if (err) {
      console.log('[Erro] ao procurar nó no banco de dados: '+err);
      throw err;
    }
    if (node_data) {
      node.update({'hw_address':device.hw_address.toLowerCase()},{
        'value'      : device.value
      },function(err,node_data){
        if (err) {
          console.log('[Erro] ao atualizar nó no banco de dados: '+err);
        }
        // else {
        //  console.log('[MongoDB] Sucesso ao atualizar nó no banco de dados: [IPv6 Device]['+device.hw_address.toLowerCase()+']');
        // }
      });
    }
    // else
    //   console.log('[MongoDB] Nó ainda não cadastrado no banco de dados');
  });
}

function verifyInt(data,node){
  var partNode = node.split(':')[5],
      stringData = data.split('&')[1],
      hashData;
  hashData = data.split('&');   // h=xxx&switch=0
  hashData = hashData[0].split('=');
  partNode = partNode.toUpperCase();
  var hashTocheck = partNode+'&'+stringData;
  var ok = sha256(hashTocheck,{ asBytes: true });
  var hashFinal = ok[0].toString(16)+ok[31].toString(16);
  console.log('[CoAP] Verificação de hash:'.red.bgBlack+'['+hashData[1]+'] --- ['+ok[0].toString(16)+ok[31].toString(16)+']');
  if (hashFinal == hashData[1]){
    console.log("[CoAP] Mensagem Integra recebida!".red.bgBlack);
    return true;
  }
  else
    console.log("[CoAP] Problema de integridade na mensagem recebida".red.bgBlack);
    return false;
}

function coapReqHandle( res ){
  // console.log(JSON.stringify(res, null, 4));
  var dev_address = res.rsinfo.address,
      payload = res.payload;

  for (var i = 0; i < devices.length; i++){
    var kj_1 = devices[i].ipv6_global.split(':'),
        kj_2 = dev_address.split(':');
    if (kj_1[5] == kj_2[5])  break;
  }
  devices[i].value = payload;
  console.log('[CoAP] Mensagem recebida:'.red.bgBlack+' ['+dev_address+'] -> '+payload);
  console.log('[CoAP] DEVICE:'.white.bgBlack+' ['+devices[i].hw_address+']');

  if (verifyInt(String(payload),dev_address)) {
    dataUpdateDB(devices[i]);
    updateDash(devices[i]);
  }
}

function coapErrorHandle( error ) {
  // console.error('[ERRO COAP]'+error );
}

function scan_coapDevices(){
  setTimeout(scan_coapDevices, constants.timeScandevices);
  console.log('[CoAP]'.red.bgBlack+' Requisitando dados...');
  if (devices.length > 0) {
    if (devices[global_devices_index].global_ipv6 != '---' &&
        devices[global_devices_index].type_device != '---' &&
        devices[global_devices_index].active != false) {
      switch (devices[global_devices_index].type_device) {
        case 'device_water':
          req = coap.request({host:devices[global_devices_index].ipv6_global,
          pathname:'/water',
          method:'GET',
          confirmable:false});
          req.on('response',coapReqHandle);
          req.on('error',coapErrorHandle);
          req.setOption('Content-Format', 'text/plain');
          req.end();
        break;
        case 'device_light':
          req = coap.request({host:devices[global_devices_index].ipv6_global,
          pathname:'/light',
          method:'GET',
          confirmable:false});
          req.on('response',coapReqHandle);
          req.on('error',coapErrorHandle);
          req.setOption('Content-Format', 'text/plain');
          req.end();
        break;
        case 'device_switch':
          req = coap.request({host:devices[global_devices_index].ipv6_global,
          pathname:'/switch',
          method:'GET',
          confirmable:false});
          req.on('response',coapReqHandle);
          req.on('error',coapErrorHandle);
          req.setOption('Content-Format', 'text/plain');
          req.end();
        break;
        default:
      }
    }
    else {
      if (devices[global_devices_index].active == false) {
        console.log('[CoAP]'.red.bgBlack+' Dispositivo offline ['+devices[global_devices_index].hw_address+']');
        dash_dev.update({'hw_assoc':devices[global_devices_index].hw_address.toLowerCase()},
        { 'active'      : false },
        function(err,node_data){
          if (err) {
            console.log('[Erro] ao atualizar dash no banco de dados: '+err);
          }
          // else {
          //  console.log('[MongoDB] Sucesso ao atualizar nó no banco de dados: [IPv6 Device]['+device.hw_address.toLowerCase()+']');
          // }
        });
      }
    }
  }

  if (global_devices_index < (devices.length-1))
    global_devices_index++;
  else
    global_devices_index = 0;
}

function get_devicesDB(){
  setTimeout(get_devicesDB, constants.timeDBdevices);
  node.find({}, function (err, nodes) {
    devices = nodes;
  });
  // console.log(devices);
}

function deleteDash(){
  dash_dev.remove({}, function (err, dash) {
  });
}

var init_coap = function(){
  deleteDash();
  setTimeout(get_devicesDB, constants.timeDBdevices);
  setTimeout(scan_coapDevices, constants.timeScandevices);
}

exports.init_coap = init_coap;
