// app/routes.js
var node = require('../backend/models/nodes');
var dev_dash = require('../backend/models/dash');
var coap = require('coap');
var sha256 = require('sha256');

module.exports = function(app, passport) {

    // =====================================
    // HOME PAGE (with login links) ========
    // =====================================
    app.get('/', function(req, res) {
        res.render('../dist/pages/login/login.ejs', { message: req.flash('loginMessage') }); // load the index.ejs file
    });

    // =====================================
    // LOGIN ===============================
    // =====================================
    // show the login form
    app.get('/login', function(req, res) {
        // render the page and pass in any flash data if it exists
        res.render('../dist/pages/login/login.ejs', { message: req.flash('loginMessage') });
    });

    // process the login form
    app.post('/login', passport.authenticate('local-login', {
        successRedirect : '/main', // redirect to the secure profile section
        failureRedirect : '/login', // redirect back to the signup page if there is an error
        failureFlash : true // allow flash messages
    }));

    // =====================================
    // SIGNUP ==============================
    // =====================================
    // show the signup form
    app.get('/signup', function(req, res) {
        // render the page and pass in any flash data if it exists
        res.render('../dist/pages/login/signup.ejs', { message: req.flash('signupMessage') });
    });

    app.post('/signup', passport.authenticate('local-signup', {
        successRedirect : '/login', // redirect to the secure profile section
        failureRedirect : '/signup', // redirect back to the signup page if there is an error
        failureFlash : true // allow flash messages
    }));

    // =====================================
    // LOGOUT ==============================
    // =====================================
    app.get('/logout', function(req, res) {
        req.logout();
        res.redirect('/');
    });

    app.get('/main', isLoggedIn, function(req, res) {
        res.render('../dist/index.ejs', {
            user : req.user // get the user out of session and pass to template
        });
    });

    // [HomeStark] Start our routes from SPA
    // =====================================
    // MAIN SECTION   ======================
    // =====================================
    // Main routes for mongodb

    // Get the devices to list
    app.get('/devices/list', function(req, res) {
      node.find({}, function (err, nodes) {
        res.send(nodes);
      });
    });

    // Get the dash sensors to list in dashboard
    app.get('/dash/list', function(req, res) {
      dev_dash.find({}, function (err, dash) {
        res.send(dash);
      });
    });

    // Set data from dashboard
    app.post('/dash/setData', function(req, res) {
      var temp = req.body;
      // console.log(temp.hw_assoc);

      switch (temp.type) {
        case 'switch':
          changeSwitch(temp);
          break;
        case 'toggle':
          toggleSensor(temp);
        default:
      }
      res.send('ok');
    });

    // Get data of sensor in dash
    app.post('/dash/GetSensor', function(req, res) {
      var temp = req.body;
      // console.log(temp.hw);
      node.findOne({'hw_address':temp.hw},function(err,sensor){
          if (sensor)
            res.send(sensor);
          else
            res.send('Não existe');
      });
    });

    // Save name of sensor
    app.post('/dash/SaveDash', function(req, res) {
      var temp = req.body;
      // console.log(temp.hw);
      dev_dash.update({ $and : [
                      {'hw_assoc':temp.hw},
                      {'type':temp.type}]},
      { 'name' : temp.name }, function(err,node_data){
        if (err) {
          console.log('[Erro] ao atualizar dash no banco de dados: '+err);
        }
      });
    });
};

function toggleSensor(data){
  node.findOne({'hw_address':data.hw_assoc}, function(err,node_data){
    if (node_data) {
      var req = coap.request({host:node_data.ipv6_global,
                              pathname:'/test/hello',
                              method:'GET',
                              confirmable:false});
      req.setOption('Content-Format', 'text/plain');
      req.end();
    }
  });
}

function n(n){
    return n > 9 ? "" + n: "0" + n;
}

function changeSwitch(data){
  node.findOne({'hw_address':data.hw_assoc}, function(err,node_data){
    if (node_data) {
      var req = coap.request({host:node_data.ipv6_global,
                              pathname:'/'+data.type,
                              method:'POST',
                              confirmable:false});
      if (data.value === true)
        data.value = 1;
      else
        data.value = 0;
      var node_s = node_data.ipv6_global.split(':')[5];
      node_s = node_s.toUpperCase();
      var hashTomount = node_s+'&switch_value='+data.value;
      var hashReady = sha256(hashTomount,{ asBytes: true });
      var message = 'h='+hashReady[0].toString(16)+hashReady[31].toString(16)+'&switch_value='+data.value;
      req.write(message);
      console.log("[CoAP] Mensagem enviado ao nó:".red.bgBlack+'['+ node_data.ipv6_global+'] --> '+message);
      req.setOption('Content-Format', 'text/plain');
      req.end();
    }
  });
}

// route middleware to make sure a user is logged in
function isLoggedIn(req, res, next) {

    // if user is authenticated in the session, carry on
    if (req.isAuthenticated())
        return next();

    // if they aren't redirect them to the home page
    res.redirect('/');
}
