<?php

/**
 * @file
 * Farm Area hooks implemented by the Farm Forest module.
 */

/**
 * Implements hook_farm_area_type_info().
 */
function farm_forest_farm_area_type_info() {
  return array(
    'plantation' => array(
      'label' => t('Plantation'),
      'style' => 'farm_map_style_green',
      'weight' => 10,
    ),
    'stand' => array(
      'label' => t('Natural Stand'),
      'style' => 'farm_map_style_dark_green',
      'weight' => 20,
    ),
  );
}
