<?php
/**
 * @file
 * Farm Map hooks implemented by the Farm Forest module.
 */

/**
 * Implements hook_farm_map_behaviors().
 */
function farm_forest_farm_map_behaviors() {
  return array(
    'select_forest_area' => array(
      'js' => 'js/farmOS.map.behaviors.select_forest_area.js',
    ),
  );
}

/**
 * Implements hook_farm_map_view().
 */
function farm_forest_farm_map_view($name, $element) {

  // Add forest areas map behaviors.
  if ($name == 'farm_forest_areas') {
    farm_map_add_behavior('popup');
    farm_map_add_behavior('areas', array('zoom' => TRUE));
    farm_map_add_behavior('select_forest_area');
  }
}
