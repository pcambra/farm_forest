<?php

/**
 * @file
 * farm_forest.features.field_instance.inc
 */

/**
 * Implements hook_field_default_field_instances().
 */
function farm_forest_field_default_field_instances() {
  $field_instances = array();

  // Exported field_instance: 'farm_plan-forest_plantation-field_farm_area'.
  $field_instances['farm_plan-forest_plantation-field_farm_area'] = array(
    'bundle' => 'forest_plantation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => 'What areas are part of this plan?',
    'display' => array(
      'default' => array(
        'label' => 'inline',
        'module' => 'taxonomy',
        'settings' => array(),
        'type' => 'taxonomy_term_reference_link',
        'weight' => 2,
      ),
    ),
    'entity_type' => 'farm_plan',
    'field_name' => 'field_farm_area',
    'label' => 'Areas',
    'required' => 0,
    'settings' => array(
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'options',
      'settings' => array(),
      'type' => 'options_select',
      'weight' => 9,
    ),
  );

  // Exported field_instance: 'farm_plan-forest_plantation-field_farm_files'.
  $field_instances['farm_plan-forest_plantation-field_farm_files'] = array(
    'bundle' => 'forest_plantation',
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'file',
        'settings' => array(),
        'type' => 'file_table',
        'weight' => 6,
      ),
    ),
    'entity_type' => 'farm_plan',
    'field_name' => 'field_farm_files',
    'label' => 'Files',
    'required' => 0,
    'settings' => array(
      'description_field' => 1,
      'file_directory' => 'farm/plan/forest_plantation',
      'file_extensions' => '',
      'max_filesize' => '',
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'multiupload_filefield_widget',
      'settings' => array(
        'progress_indicator' => 'bar',
      ),
      'type' => 'file_mfw',
      'weight' => 5,
    ),
  );

  // Exported field_instance: 'farm_plan-forest_plantation-field_farm_notes'.
  $field_instances['farm_plan-forest_plantation-field_farm_notes'] = array(
    'bundle' => 'forest_plantation',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'inline',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 5,
      ),
    ),
    'entity_type' => 'farm_plan',
    'field_name' => 'field_farm_notes',
    'label' => 'Notes',
    'required' => 0,
    'settings' => array(
      'text_processing' => 1,
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'rows' => 5,
      ),
      'type' => 'text_textarea',
      'weight' => 4,
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('Areas');
  t('Files');
  t('Notes');
  t('What areas are part of this plan?');

  return $field_instances;
}
