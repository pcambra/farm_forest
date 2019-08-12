<?php

/**
 * @file
 * farm_forest.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function farm_forest_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "field_group" && $api == "field_group") {
    return array("version" => "1");
  }
}

/**
 * Implements hook_views_api().
 */
function farm_forest_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}

/**
 * Implements hook_default_farm_plan_type().
 */
function farm_forest_default_farm_plan_type() {
  $items = array();
  $items['forest'] = entity_import('farm_plan_type', '{
    "type" : "forest",
    "label" : "Forest Plan",
    "weight" : 0,
    "rdf_mapping" : []
  }');
  return $items;
}
