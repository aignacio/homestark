$( document ).ready(function() {
  $(".sidebar-brand").click(function(e) {
    e.preventDefault();
    $("#wrapper").toggleClass("toggled");
    $("#wrapper").toggleClass("untoggled");
  });
});
