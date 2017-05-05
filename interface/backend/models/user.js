var mongoose = require('mongoose');
var bcrypt   = require('bcrypt-nodejs');

var userSchema = new mongoose.Schema({
  profile: {
    admin: {
      type: Boolean
      // ,required: true
    }
    ,username: {
      type: String
      ,required: true
      ,lowercase: true
      ,unique: true
    }
    ,firstname: {
      type: String
      // ,required: true
    }
    ,lastname: {
      type: String
      // ,required: true
    }
    ,password: {
      type: String
      ,required: true
    }
    ,mail:{
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
  },
  data: {
    oauth: {
      type: String
    }
    ,apperance: {
      menuColor: String
      ,mainColor: String
    }
    ,csvConfig: {
      type: String
      // ,required: true
      ,match: /^http:\/\//i
    }
  }
});

// methods ======================
// generating a hash
userSchema.methods.generateHash = function(password) {
    return bcrypt.hashSync(password, bcrypt.genSaltSync(8), null);
};

// checking if password is valid
userSchema.methods.validPassword = function(password) {
    return bcrypt.compareSync(password, this.profile.password);
};

// create the model for users and expose it to our app
module.exports = mongoose.model('User', userSchema);
