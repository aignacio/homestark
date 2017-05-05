var hero = angular.module('superhero', ['ui.bootstrap',
                                        'ngRoute',
                                        'homeStarkControllers',
                                        'xeditable',
                                        'ngAnimate']);
hero.config(['$controllerProvider',
  function($controllerProvider) {
    $controllerProvider.allowGlobals();
  }
]);

hero.run(['editableOptions',function(editableOptions) {
  editableOptions.theme = 'bs3';
}]);

hero.config(['$routeProvider',function($routeProvider) {
  $routeProvider.
    when('/dash', {
      templateUrl: '/pages/windows/dashboard.ejs',
      controller: ''
    }).
    when('/devices', {
      templateUrl: '/pages/windows/devices.ejs',
      controller: ''
    }).
    when('/node_map', {
      templateUrl: '/pages/windows/node_map.ejs',
      controller: ''
    }).
    when('/about', {
      templateUrl: '/pages/windows/about.ejs',
      controller: ''
    }).
    otherwise({
      redirectTo: '/dash',
      controller: ''
    });
}]);
