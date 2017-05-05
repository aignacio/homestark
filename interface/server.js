// server.js
// set up ======================================================================
// get all the tools we need
var express   = require('express');
var app       = express();
var port      = process.env.PORT || 8888;
var mongoose  = require('mongoose');
var passport  = require('passport');
var flash     = require('connect-flash');
var mongoCon  = require('./config/database');
var path      = require('path');
var snmp_scan = require('./backend/scripts/snmp_scan');
var coap_req  = require('./backend/scripts/coap_req');

var morgan       = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser   = require('body-parser');
var session      = require('express-session');

var configDB = require('./config/database.js');

app.use(bodyParser.json({limit: '50mb'}));
app.use(bodyParser.urlencoded({limit: '50mb', extended: true}));

// configuration ===============================================================
var DEBUG_MONGO = 0;
mongoCon(DEBUG_MONGO);

require('./config/passport')(passport); // pass passport for configuration

// set up our express application
app.use(morgan('dev')); // log every request to the console
app.use(cookieParser()); // read cookies (needed for auth)
app.use(bodyParser()); // get information from html forms

app.set('view engine', 'ejs'); // set up ejs for templating

// required for passport
app.use(session({ secret: '3102230' })); // session secret
app.use(passport.initialize());
app.use(passport.session()); // persistent login sessions
app.use(flash()); // use connect-flash for flash messages stored in session
app.use(express.static(path.join(__dirname, 'dist')));
console.log(path.join(__dirname, 'dist'));

// app.use(express.static(__dirname + 'public'));

// routes ======================================================================
require('./backend/routes.js')(app, passport); // load our routes and pass in our app and fully configured passport

// launch scripts & express ====================================================
snmp_scan.init_snmp_can();
coap_req.init_coap();

app.listen(port);
console.log('[HomeStark] Server open on port:' + port);
