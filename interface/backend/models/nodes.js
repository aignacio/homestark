var mongoose = require('mongoose');

var nodesSchema = new mongoose.Schema({
    hw_address: {
      type: String
      ,required: true
      ,lowercase: true
      ,unique: true
    }
    ,active: {
      type: Boolean
      ,required: true
    }
    ,type_device: {
      type: String
    }
    ,rssi_value: {
      type: String
    }
    ,rota_pref: {
      type: String
    }
    ,rank_rpl:{
      type: String
    }
    ,link_metric:{
      type: String
    }
    ,ipv6_global:{
      type: String
    }
    ,ipv6_local:{
      type: String
    }
    ,value:{
      type: String
    }
    ,created: {
      type: Date
      ,default: Date.now
    }
    ,updated: {
      type: Date
      ,default: Date.now
    }
});

// methods ======================

// create the model for users and expose it to our app
module.exports = mongoose.model('node', nodesSchema);
