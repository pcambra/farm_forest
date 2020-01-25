(function () {
  farmOS.map.behaviors.select_forest_area = {
    attach: function (instance) {

      // When an area popup is displayed, copy the area to the area selection
      // form field.
      instance.popup.on('farmOS-map.popup', function (event) {
        var area_name = jQuery('.ol-popup-name a').html();
        if (area_name) {
          jQuery('#edit-area').val(area_name);
        }
      });
    }
  };
}());
