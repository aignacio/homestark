var homeStarkController = angular.module('homeStarkControllers', ['ui.bootstrap',
                                                                  'ngRoute',
                                                                  'gridster',
                                                                  'ui.knob',
                                                                  'uiSwitch',
                                                                  'angular-theme-spinner']);

homeStarkController.controller('loginCtrl',['$scope',function ($scope) {
  $scope.loadLogin = function(){
    $scope.class = "loading";
  };

}]);

homeStarkController.controller('devicesCtrl',['$scope','$http','$timeout',function ($scope,$http,$timeout) {
  // $scope.rowCollection = [
  //   {firstName: 'Laurent', lastName: 'Renard', birthDate: new Date('1987-05-21'), balance: 102, email: 'whatever@gmail.com'},
  //   {firstName: 'Blandine', lastName: 'Faivre', birthDate: new Date('1987-04-25'), balance: -2323.22, email: 'oufblandou@gmail.com'},
  //   {firstName: 'Francoise', lastName: 'Frere', birthDate: new Date('1955-08-27'), balance: 42343, email: 'raymondef@gmail.com'}
  // ];
  $scope.rowCollection = [];
  $scope.displayedCollection = [];
  $scope.displayedCollection = [].concat($scope.rowCollection);

  $scope.$on("$destroy",function(){
      $timeout.cancel(refreshDevices);
  });

  var refreshDevices = function() {
    time = $timeout(refreshDevices, 30000);
    $http({method: 'GET', url: '/devices/list'}).
      then(function(response) {
        for(var i=0;i<response.data.length;i++) {
          // alert(response.data[i].active);
          if(response.data[i].active === true)
            response.data[i].active = 'Ativo';
          else
            response.data[i].active = 'Inativo';
        }
        $scope.rowCollection = response.data;
        // console.log($scope.users);
      }, function(response) {
        $scope.data = response.data || "Request failed";
        $scope.status = response.status;
        console.log($scope.status);
    });
  };

  refreshDevices();
}]);

homeStarkController.controller('dashCtrl',['$scope','ShareDash','$uibModal','$timeout','$http',function($scope,ShareDash,$uibModal,$timeout,$http){
  $scope.gridsterOpts = {
      columns: 4, // the width of the grid, in columns
      pushing: true, // whether to push other items out of the way on move or resize
      floating: true, // whether to automatically float items up so they stack (you can temporarily disable if you are adding unsorted items with ng-repeat)
      swapping: true, // whether or not to have items of the same size switch places instead of pushing down if they are the same size
      width: 'auto', // can be an integer or 'auto'. 'auto' scales gridster to be the full width of its containing element
      colWidth: 300, // can be an integer or 'auto'.  'auto' uses the pixel width of the element divided by 'columns'
      rowHeight: 'match', // can be an integer or 'match'.  Match uses the colWidth, giving you square widgets.
      margins: [10, 10], // the pixel distance between each widget
      outerMargin: true, // whether margins apply to outer edges of the grid
      sparse: false, // "true" can increase performance of dragging and resizing for big grid (e.g. 20x50)
      isMobile: false, // stacks the grid items if true
      mobileBreakPoint: 600, // if the screen is not wider that this, remove the grid layout and stack the items
      mobileModeEnabled: true, // whether or not to toggle mobile mode when screen width is less than mobileBreakPoint
      minColumns: 1, // the minimum columns the grid must have
      minRows: 2, // the minimum height of the grid, in rows
      maxRows: 100,
      defaultSizeX: 2, // the default width of a gridster item, if not specifed
      defaultSizeY: 1, // the default height of a gridster item, if not specified
      minSizeX: 1, // minimum column width of an item
      maxSizeX: null, // maximum column width of an item
      minSizeY: 1, // minumum row height of an item
      maxSizeY: null, // maximum row height of an item
      resizable: {
         enabled: true,
         handles: ['n', 'e', 's', 'w', 'ne', 'se', 'sw', 'nw'],
         start: function(event, $element, widget) {}, // optional callback fired when resize is started,
         resize: function(event, $element, widget) {}, // optional callback fired when item is resized,
         stop: function(event, $element, widget) {} // optional callback fired when item is finished resizing
      },
      draggable: {
         enabled: true // whether dragging items is supported
        //  handle: '.my-class', // optional selector for drag handle
        //  start: function(event, $element, widget) {}, // optional callback fired when drag is started,
        //  drag: function(event, $element, widget) {}, // optional callback fired when item is moved,
        //  stop: function(event, $element, widget) {} // optional callback fired when item is finished dragging
      }
  };

  $scope.dash_sensors = [
    // { sizeX: 1, sizeY: 1},
    // { sizeX: 1, sizeY: 1},
    // { sizeX: 1, sizeY: 1}
    // { sizeX: 1, sizeY: 1},
    // { sizeX: 1, sizeY: 1},
    // { sizeX: 1, sizeY: 1},
    // { sizeX: 1, sizeY: 1},
    // { sizeX: 1, sizeY: 1},
  ];

  $scope.options_water = {
    readOnly: true,
    unit:"%",
    subText: {
      enabled: true,
      text: 'Nível',
      color: 'gray'
    },
    trackWidth: 30,
    barWidth: 30,
    step: 1,
    trackColor: 'rgba(52,152,219,.1)',
    barColor: 'rgba(52,152,219,.5)'
  };

  $scope.options_light = {
    readOnly: true,
    subText: {
      enabled: true,
      text: 'Lux',
      color: 'gray'
    },
    bgColor: '#2C3E50',
    trackWidth: 50,
    barWidth: 30,
    max: 2000,
    barColor: '#FFAE1A',
    textColor: '#eee'
  };

  $scope.$on("$destroy",function(){
      $timeout.cancel(refreshSensors);
  });

  $scope.$on('$routeChangeStart', function(scope, next, current){
    $timeout.cancel(refreshSensors);
  });

  $scope.toggleSensor = function(bt_st,hw,type_c){
    // console.log('BOTAO:'+bt_st+' HW:'+hw+' Tipo:'+type);
    var data ={hw_assoc:hw,
               type:'toggle'};
    $http({
      url: '/dash/setData', // No need of IP address
      method: 'POST',
      data: data,
      headers: {'Content-Type': 'application/json'}
    }).then(function(response) {
      // if(response.data.status=='ok'){
      //  console.log("Valor da chave enviada ao sensor!");
      // }
      // else
      //   console.log("Não foi possível enviar o valor da chave!");
    }, function(response) {
      $scope.data = response.data || "Request failed";
      $scope.status = response.status;
      console.log($scope.status);
    });
    // alert('BOTAO:'+bt_st+' HW:'+hw+' Tipo:'+type);
  };

  $scope.toggleSwitch = function(bt_st,hw,type_c){
    // console.log('BOTAO:'+bt_st+' HW:'+hw+' Tipo:'+type);
    var data ={hw_assoc:hw,
               value:bt_st,
               type:type_c};
    $http({
      url: '/dash/setData', // No need of IP address
      method: 'POST',
      data: data,
      headers: {'Content-Type': 'application/json'}
    }).then(function(response) {
      // if(response.data.status=='ok'){
      //  console.log("Valor da chave enviada ao sensor!");
      // }
      // else
      //   console.log("Não foi possível enviar o valor da chave!");
    }, function(response) {
      $scope.data = response.data || "Request failed";
      $scope.status = response.status;
      console.log($scope.status);
    });
    // alert('BOTAO:'+bt_st+' HW:'+hw+' Tipo:'+type);
  };

  function processSensors(sensors){
    var sensor_dash = [];
    for (var i = 0; i < sensors.length; i++) {
      if (sensors[i].type == 'switch')
        if (sensors[i].value == '0')
          sensors[i].value = false;
        else
          sensors[i].value = true;

      sensor_dash.push({status:sensors[i].active,
                        sizeX: 1,
                        sizeY: 1,
                        type: sensors[i].type,
                        value: sensors[i].value,
                        name: sensors[i].name,
                        updated: sensors[i].updated,
                        hw: sensors[i].hw_assoc});

    }
    // console.log(sensor_dash);
    return sensor_dash;
  }

  function UpdateSensors(data){
    for (var i = 0; i < data.length; i++) {
      if (data[i].type == 'switch')
        if (data[i].value == '0')
          data[i].value = false;
        else
          data[i].value = true;
      // if (data[i].active === false)
      //   $scope.dash_sensors[i].status = 'Inativo';
      // else
      //   $scope.dash_sensors[i].status = 'Ativo';

      $scope.dash_sensors[i].status = data[i].active;
      $scope.dash_sensors[i].value = data[i].value;
      $scope.dash_sensors[i].updated = data[i].updated;
    }
  }

  var cancelUpdate = function(){
      $scope.showUpdate = false;
  };

  var refreshSensors = function() {
    time = $timeout(refreshSensors,5000);
    $scope.showUpdate = true;
    $http({method: 'GET', url: '/dash/list'}).
      then(function(response) {
        // for(var i=0;i<response.data.length;i++) {
        //   if(response.data[i].active === true)
        //     response.data[i].active = 'Ativo';
        //   else
        //     response.data[i].active = 'Inativo';
        // }
        $timeout(cancelUpdate,3000);
        if (response.data.length > $scope.dash_sensors.length) {
          $scope.dash_sensors = processSensors(response.data);
        }
        else {
          UpdateSensors(response.data);
        }
        // console.log($scope.users);
      }, function(response) {
        $scope.data = response.data || "Request failed";
        $scope.status = response.status;
        console.log($scope.status);
    });
  };

  refreshSensors();

  $scope.dash = ShareDash;

  $scope.editDash = function (hw,type,name,value,status,updated) {
    $scope.dash.hw = hw;
    $scope.dash.type = type;
    $scope.dash.name = name;
    $scope.dash.value = value;
    $scope.dash.status = status;
    $scope.dash.updated = updated;

    var modalInstance = $uibModal.open({
      size:'lg',
      animation: true,
      templateUrl: 'modal.html',
      controller: 'modalDashCtrl'
    });
  };

}]);

homeStarkController.controller('modalDashCtrl', ['$scope','$timeout','$uibModalInstance','ShareDash','$http','$location', function ($scope, $timeout,$uibModalInstance, ShareDash, $http,$location) {
  $scope.dash_sensor = ShareDash;

  $http({ url: '/dash/GetSensor', // No need of IP address
          method: 'POST',
          data:{'hw':$scope.dash_sensor.hw},
          headers: {'Content-Type': 'application/json'}
  }).then(function(response) {
      if(response.data.active === true)
        response.data.active = 'Ativo';
      else
        response.data.active = 'Inativo';
      $scope.sensor = response.data;
      // console.log('Sensor:'+$scope.sensor);
    }, function(response) {
      $scope.data = response.data || "Request failed";
      $scope.status = response.status;
      console.log($scope.status);
  });

  $scope.saveCfg = function (){
    $http({ url: '/dash/SaveDash',
            method: 'POST',
            data:{name:$scope.dash_sensor.name,type:$scope.dash_sensor.type,hw:$scope.dash_sensor.hw},
            headers: {'Content-Type': 'application/json'}
    }).then(function(response) {
      console.log("Alterações salvas!");
      }, function(response) {
        $scope.data = response.data || "Request failed";
        $scope.status = response.status;
        console.log($scope.status);
    });
  };

  $scope.cancel = function () {
    $uibModalInstance.dismiss();
  };
}]);

homeStarkController.service('ShareDash', function () {
    var dash = {
      hw:'',
      type:'',
      value:'',
      name:'',
      status:'',
      updated:''
    };

    return dash;
});

homeStarkController.controller('networkCtrl', ['$scope','$timeout','$http','$location', function ($scope,$timeout, $http,$location) {
  // create an array with nodes
  var nodes = [{
    id: 'aa04',
    label: 'aa04',
    group:'border_router'
  }];
  var edges = [];
  var options = {
      interaction:{
        hover:true
      },
      nodes: {
          color: 'black',
          shape: 'dot',
          size: 30,
          font: {
              size: 32
          },
          borderWidth: 0,
          shadow:true
      },
      edges: {
          width: 2,
          shadow:true
      },
      groups: {
        device_switch: {
          shape: 'icon',
          icon: {
            face: 'FontAwesome',
            code: '\uf204',
            size: 50,
            color: 'black'
          }
        },
        device_water: {
          shape: 'icon',
          icon: {
            face: 'FontAwesome',
            code: '\uf043',
            size: 50,
            color: 'black'
          }
        },
        device_light: {
          shape: 'icon',
          icon: {
            face: 'FontAwesome',
            code: '\uf0eb',
            size: 50,
            color: 'black'
          }
        },
        border_router:{
          shape: 'icon',
          icon:{
            face:'FontAwesome',
            code:'\uf17c',
            size: 50,
            color: 'black'
          }
        }
      }
  };
  var container = document.getElementById('networkNodes');
  var data = {
    nodes: nodes,
    edges: edges
  };
  var network;

  $http({method: 'GET', url: '/devices/list'}).
    then(function(response) {
      createNodes(response.data);
      network = new vis.Network(container, data, options);
      network.on("doubleClick", function (params) {
        console.log(params);
        $(this.document).popover();
      });
    }, function(response) {
      $scope.data = response.data || "Request failed";
      $scope.status = response.status;
      console.log($scope.status);
  });

  function createNodes(node_db){
    for (var i = 0; i < node_db.length; i++) {
      if (node_db[i].ipv6_local != '---' && node_db[i].rota_pref != '---') {
        nodes.push({
          id: node_db[i].ipv6_local.split(':')[5],
          label: node_db[i].ipv6_local.split(':')[5],
          group:node_db[i].type_device
        });

        edges.push({
          from:node_db[i].ipv6_local.split(':')[5],
          to:node_db[i].rota_pref.split(':')[5],
          label: node_db[i].rssi_value,
          font: {strokeWidth: 2}
        });
      }
    }
  }
}]);
