<?php

/**
 * @file
 * Farm forest plan planting form.
 */

/**
 * Forest plan planting form.
 */
function farm_forest_plan_planting_form($form, &$form_state, $plan) {

  // Start an empty form array.
  $form = array();

  // Set the page title.
  drupal_set_title('Planting');

  // Load area(s) associated with this plan.
  $area_ids = farm_plan_linked_records('area', $plan->id);

  // If the plan does not have any areas, show a message and bail.
  if (empty($area_ids)) {
    drupal_set_message(t('Areas must be assigned to this plan before a planting can be created.'), 'warning');
    return $form;
  }

  // Treat the form as a tree (no pun intended).
  $form['#tree'] = TRUE;

  // Add the plan to the form values, for use in form submission.
  $form['plan'] = array(
    '#type' => 'value',
    '#value' => $plan,
  );

  // Load asset(s) associated with this plan.
  $asset_ids = farm_plan_linked_records('asset', $plan->id);
  $assets = farm_asset_load_multiple($asset_ids);

  // Iterate through the assets to make a list of plantings.
  $plantings = array();
  foreach ($assets as $asset) {

    // If the asset is not a planting, skip it.
    if ($asset->type != 'planting') {
      continue;
    }

    // Add it to the list.
    $plantings[] = $asset;
  }

  // Iterate through the plantings and build forms.
  foreach ($plantings as $planting) {

    // Add a planting details form.
    $form['plantings'][$planting->id] = farm_forest_plan_planting_details_form($planting);

    // Make it a fieldset with a title.
    $form['plantings'][$planting->id]['#type'] = 'fieldset';
    $form['plantings'][$planting->id]['#title'] = t('Planting') . ': ' . entity_label('farm_asset', $planting);

    // Add the planting to the form values, for use in form submission.
    $form['plantings'][$planting->id]['planting'] = array(
      '#type' => 'value',
      '#value' => $planting,
    );
  }

  // Add a blank form for creating a new planting.
  $form['new'] = farm_forest_plan_planting_details_form();
  $form['new']['#type'] = 'fieldset';
  $form['new']['#title'] = t('Create a planting');

  // If there are plantings already, collapse the new planting form.
  if (!empty($plantings)) {
    $form['new']['#collapsible'] = TRUE;
    $form['new']['#collapsed'] = TRUE;
  }

  // Submit button for saving all changes.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  // Return the form.
  return $form;
}

/**
 * Planting details sub-form.
 *
 * @param $planting
 *   The planting asset, or NULL.
 *
 * @return array
 *   Returns a form array that can be used in another form.
 *
 * @see farm_forest_plan_planting_form().
 */
function farm_forest_plan_planting_details_form($planting=NULL) {

  // Define the date format.
  $date_format = 'Y-m-d';

  // Define default values.
  $defaults = array(
    'species' => '',
    'date' => REQUEST_TIME,
    'count' => '',
    'spacing' => '',
  );

  // If a planting is provided, load default values.
  $transplanting_log = NULL;
  if (!empty($planting)) {

    // Get the planting metadata wrapper.
    $planting_wrapper = entity_metadata_wrapper('farm_asset', $planting);

    // Get the species name.
    $defaults['species'] = $planting_wrapper->field_farm_crop->label();

    // Find transplanting logs that reference the planting.
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'log');
    $query->entityCondition('bundle', 'farm_transplanting');
    $query->fieldCondition('field_farm_asset', 'target_id', $planting->id);
    $query->propertyOrderBy('id', 'ASC');
    $result = $query->execute();
    if (isset($result['log'])) {
      $log_ids = array_keys($result['log']);
      $log_id = reset($log_ids);
      $transplanting_log = log_load($log_id);
    }

    // If a log was found, get default values from it.
    if (!empty($transplanting_log)) {

      // Get the date.
      $defaults['date'] = $transplanting_log->timestamp;

      // Get the count.
      $count_measure = farm_quantity_log_data($transplanting_log, 'count', t('total planted'));
      if (!empty($count_measure[0]['value'])) {
        $defaults['count'] = $count_measure[0]['value'];
      }

      // Get the spacing.
      $spacing_measure = farm_quantity_log_data($transplanting_log, 'length', t('tree spacing'));
      if (!empty($spacing_measure[0]['value'])) {
        $defaults['spacing'] = $spacing_measure[0]['value'];
      }
    }
  }

  // Tree species.
  $form['species'] = array(
    '#type' => 'textfield',
    '#title' => t('Tree species'),
    '#autocomplete_path' => 'taxonomy/autocomplete/field_farm_crop',
    '#default_value' => $defaults['species'],
  );

  // Transplanting log.
  $form['transplanting']['log'] = array(
    '#type' => 'value',
    '#value' => $transplanting_log,
  );

  // Transplanting date.
  $form['transplanting']['date'] = array(
    '#type' => 'date_select',
    '#title' => t('Planting date'),
    '#date_format' => $date_format,
    '#date_label_position' => 'within',
    '#date_year_range' => '-3:+10',
    '#default_value' => date($date_format, $defaults['date']),
  );

  // Total trees planted.
  $form['transplanting']['count'] = array(
    '#type' => 'textfield',
    '#title' => t('Total trees planted'),
    '#input_group' => TRUE,
    '#field_suffix' => t('trees'),
    '#element_validate' => array('element_validate_integer_positive'),
    '#default_value' => $defaults['count'],
  );

  // Load the system of measurement.
  $system_of_measurement = farm_quantity_system_of_measurement();
  $units = 'meters';
  if ($system_of_measurement == 'us') {
    $units = 'yards';
  }

  // Tree spacing.
  $form['transplanting']['spacing'] = array(
    '#type' => 'textfield',
    '#title' => t('Tree spacing'),
    '#input_group' => TRUE,
    '#field_suffix' => t($units),
    '#element_validate' => array('element_validate_number'),
    '#default_value' => $defaults['spacing'],
  );
  $form['transplanting']['spacing_units'] = array(
    '#type' => 'hidden',
    '#value' => $units,
  );

  // Return the form array.
  return $form;
}

/**
 * Forest plan planting form validation.
 */
function farm_forest_plan_planting_form_validate($form, &$form_state) {

}

/**
 * Forest plan planting form submit.
 */
function farm_forest_plan_planting_form_submit($form, &$form_state) {

  // Get the plan.
  $plan = $form_state['values']['plan'];

  // If a new planting species and date are provided, create them.
  if (!empty($form_state['values']['new']['species']) && !empty($form_state['values']['new']['transplanting']['date'])) {

    // Create a name for the planting.
    $planting_name = t('@species planting (@plan)', array('@species' => $form_state['values']['new']['species'], '@plan' => entity_label('farm_plan', $plan)));

    // Create a new planting asset.
    $values = array(
      'type' => 'planting',
      'name' => $planting_name,
    );
    $planting_asset = entity_create('farm_asset', $values);
    $planting_wrapper = entity_metadata_wrapper('farm_asset', $planting_asset);

    // Get the species term (or create a new one), and add it to the planting.
    $species = farm_term($form_state['values']['new']['species'], 'farm_crops');
    $planting_wrapper->field_farm_crop[] = $species;

    // Save the planting.
    $planting_wrapper->save();

    // Link the planting to the plan.
    farm_plan_link_record('asset', $plan->id, $planting_asset->id);

    // Set a message.
    $label = entity_label('farm_asset', $planting_asset);
    $uri = entity_uri('farm_asset', $planting_asset);
    drupal_set_message('Planting created: ' . l($label, $uri['path']));

    // Load areas that are associated with the plan.
    $area_ids = farm_plan_linked_records('area', $plan->id);
    $areas = taxonomy_term_load_multiple($area_ids);

    // Convert the date to a timestamp.
    $timestamp = strtotime($form_state['values']['new']['transplanting']['date']);

    // If the timestamp is in the past, the log is done. Otherwise, it is not.
    $done = FALSE;
    if ($timestamp < REQUEST_TIME) {
      $done = TRUE;
    }

    // Create a new transplanting log, using the movement log helper function.
    $log = farm_movement_create($planting_asset, $areas, $timestamp, 'farm_transplanting', $done);

    // Link the log to the plan.
    farm_plan_link_record('log', $plan->id, $log->id);

    // Add quantity measurements, if available.
    $measurements = array();
    if (!empty($form_state['values']['new']['transplanting']['count'])) {
      $measurements[] = array(
        'measure' => 'count',
        'value' => $form_state['values']['new']['transplanting']['count'],
        'units' => t('trees'),
        'label' => t('total planted'),
      );
    }
    if (!empty($form_state['values']['new']['transplanting']['spacing'])) {
      $measurements[] = array(
        'measure' => 'length',
        'value' => $form_state['values']['new']['transplanting']['spacing'],
        'units' => t($form_state['values']['new']['transplanting']['spacing_units']),
        'label' => t('tree spacing'),
      );
    }

    // If there are quantity measurements, add them to the log.
    if (!empty($measurements)) {
      farm_quantity_log_add_measurements($log, $measurements);
    }
  }
}