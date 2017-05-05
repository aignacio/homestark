var bcrypt   = require('bcrypt-nodejs');
var hash = bcrypt.hashSync("abc");

console.log(hash); 
console.log(bcrypt.compareSync("bacon", hash)); // true
console.log(bcrypt.compareSync("veggies", hash)); // false
