var device_list = [];

var obj_ad1 = {
  address_ipv6:"aaaa::1",
  status:false
}

var obj_ad2 = {
  address_ipv6:"aaaa::2",
  status:false
}
device_list.push(obj_ad1);
device_list.push(obj_ad2);

Array.prototype.hasNode = function(element) {
    var i;
    for (i = 0; i < this.length; i++) {
        if (this[i].address_ipv6 === element) {
            return i; //Returns element position, so it exists
        }
    }

    return -1; //The element isn't in your array
};

console.log("Antes de remover");
for (var i in device_list) {
  console.log(device_list[i].address_ipv6);
}
device_list.splice(0,1);

console.log("Depois de remover");
for (var i in device_list) {
  console.log(device_list[i].address_ipv6);
}
console.log('TAG CONTIDA:'+device_list.hasNode('aaaa::2'));
