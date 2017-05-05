var mongoose = require('mongoose');
var timeoutRetry = 30000;

var address = '127.0.0.1';
var portmong = '27017';

var colors = require('colors/safe');

colors.setTheme({
  dimmer: 'magenta',
  current: 'green',
  pir: 'rainbow',
  ldr: 'grey',
  temperature: 'gray',
  ip: 'yellow',
  rssi: 'cyan',
  status: 'white',
  msp: 'gray',
  warn: 'yellow',
  debug: 'blue',
  error: 'red'
});

/*************************************************************************/
module.exports = function(DEBUG_MONGOOSE_MODE){

	mongoose.set('debug', DEBUG_MONGOOSE_MODE);

	var MONGO = {
		server: address,
	    port: portmong,
	    db: 'HomeStark',
	    connectionString : function(){
	    	return 'mongodb://'+this.server+':'+this.port+'/'+this.db;
	    }
	};

	var uri = MONGO.connectionString();

	var connectWithRetry = function() {
		return mongoose.connect(uri, MONGO.options, function(err) {
			if (err) {
			  console.error(colors.ip('[Mongoose]: Erro ao conectar, tentando conectar novamente em ' + timeoutRetry + ' sec', err));
			  setTimeout(connectWithRetry, timeoutRetry);
			}
		});
	};

	connectWithRetry();

	db = mongoose.connection;

	db.on('connecting', function(){
		console.log(colors.ip('[Mongoose]: Tentando conector ao MongoDB...'+uri));
	});

	db.on('connected', function(){
		console.log(colors.ip('[Mongoose]: Conectado em '+uri));
	});

	db.on('disconnected', function(){
		console.log(colors.ip('[Mongoose]: Desconectado em '+uri));
    setTimeout(connectWithRetry, timeoutRetry);
	});

	db.once('open', function() {
    	console.log(colors.ip('[Mongoose]: Conexão efetuada com sucesso: '+uri));
  	});

  	db.on('reconnected', function () {
	    console.log('[Mongoose]: Reconexão ao MongoDB: '+uri);
  	});

	db.on('error', function(a1){
		console.log(colors.ip('[Mongoose]: Erro na conexão: '+uri));
		console.log(a1);
    	mongoose.disconnect();
	});

	process.on('SIGINT', function(){
		mongoose.connection.close(function(){
			console.log(colors.ip('[Mongoose]: Desconectado pelo término da aplicação'));
			process.exit(0);
		});
	});
}
