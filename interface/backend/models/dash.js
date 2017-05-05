var mongoose = require('mongoose');

var dashSchema = new mongoose.Schema({
    position_x: {
      type: String
    }
    ,position_y: {
      type: String
    }
    ,value: {
      type: String
    }
    ,active: {
      type: Boolean
    }
    ,type: {
      type: String
    }
    ,hw_assoc: {
      type: String
    }
    ,name: {
      type: String
    }
    ,active: {
      type: String
    }
    ,updated: {
      type: Date
      ,default: Date.now
    }
});

module.exports = mongoose.model('dash', dashSchema);
