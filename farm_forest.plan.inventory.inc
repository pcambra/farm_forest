<?php

/**
 * @file
 * Farm forest plan inventory form.
 */

/**
 * Forest plan inventory form.
 */
function farm_forest_plan_inventory_form($form, &$form_state, $plan) {

  // Start a form array.
  $form = array(
    '#tree' => TRUE,
  );

  // Set the page title.
  drupal_set_title('Inventory');

  // Add the plan to the form values, for use in form submission.
  $form['plan'] = array(
    '#type' => 'value',
    '#value' => $plan,
  );

  // Load a list of inventory logs associated with this plan.
  $log_types = array(
    'farm_observation' => t('Observation'),
  );
  $logs = farm_forest_plan_logs($plan->id, array_keys($log_types));

  // Load a list of assets associated with this plan.
  $asset_ids = farm_plan_linked_records('asset', $plan->id);
  $assets = array();
  foreach ($asset_ids as $id) {
    $asset = farm_asset_load($id);
    if (!empty($asset)) {
      $assets[] = $asset;
    }
  }

  // Add fieldsets for each existing log.
  foreach ($logs as $log) {
    $form['logs'][$log->id] = farm_forest_plan_log_details_form($log, $log_types, $assets, FALSE);
    $form['logs'][$log->id]['#title'] = date('Y-m-d', $log->timestamp) . ' ' . $log_types[$log->type] . ': ' . entity_label('log', $log);

    // If the log has an ID, add a link to remove it.
    if (!empty($log->id)) {
      $form['logs'][$log->id]['remove'] = array(
        '#type' => 'markup',
        '#markup' => l(t('Remove'), 'farm/plan/' . $plan->id . '/log/' . $log->id . '/delete', array('query' => array('destination' => current_path()))),
        '#weight' => 100,
      );
    }
  }

  // Fieldset for adding a new task.
  // If there are logs already, collapse it.
  $collapsed = !empty($logs) ? TRUE : FALSE;
  $form['logs']['new'] = farm_forest_plan_log_details_form(NULL, $log_types, $assets, FALSE);
  $form['logs']['new']['#title'] = t('Record an inventory observation');
  $form['logs']['new']['#collapsed'] = $collapsed;

  // Load the system of measurement and define units.
  $system_of_measurement = farm_quantity_system_of_measurement();
  $units = array(
    'area' => 'hectares',
    'volume' => 'cubic meters',
    'length' => 'meters',
  );
  if ($system_of_measurement == 'us') {
    $units = array(
      'area' => 'acres',
      'volume' => 'cubic feet',
      'length' => 'feet',
    );
  }
  $form['units'] = array(
    '#type' => 'value',
    '#value' => $units,
  );

  // Iterate through the log fieldsets and add inventory specific fields.
  foreach (element_children($form['logs']) as $key) {

    // Load existing measurements from the log for default values.
    $defaults = farm_forest_plan_inventory_log_measurements($form['logs'][$key]['log']['#value']);

    // Area.
    $form['logs'][$key]['area'] = array(
      '#type' => 'textfield',
      '#title' => t('Area'),
      '#input_group' => TRUE,
      '#field_suffix' => t($units['area']),
      '#element_validate' => array('element_validate_integer_positive'),
      '#default_value' => $defaults['area'],
      '#weight' => 20,
    );

    // Volume.
    $form['logs'][$key]['volume'] = array(
      '#type' => 'textfield',
      '#title' => t('Volume'),
      '#input_group' => TRUE,
      '#field_suffix' => t($units['volume']),
      '#element_validate' => array('element_validate_integer_positive'),
      '#default_value' => $defaults['volume'],
      '#weight' => 20,
    );

    // Maximum height.
    $form['logs'][$key]['height'] = array(
      '#type' => 'textfield',
      '#title' => t('Maximum height'),
      '#input_group' => TRUE,
      '#field_suffix' => t($units['length']),
      '#element_validate' => array('element_validate_integer_positive'),
      '#default_value' => $defaults['height'],
      '#weight' => 20,
    );

    // MAI.
    $form['logs'][$key]['mai'] = array(
      '#type' => 'textfield',
      '#title' => t('Mean annual increment (MAI)'),
      '#input_group' => TRUE,
      '#field_suffix' => t($units['length']),
      '#element_validate' => array('element_validate_integer_positive'),
      '#default_value' => $defaults['mai'],
      '#weight' => 20,
    );

    // Silviculture score (this is added to notes, so it is only a separate
    // field for new logs).
    if ($key == 'new') {
      $score_options = array(
        'Good',
        'Usable',
        'Not usable',
      );
      $form['logs'][$key]['score'] = array(
        '#type' => 'select',
        '#title' => t('Average silviculture score'),
        '#options' => drupal_map_assoc($score_options),
        '#weight' => 20,
      );
    }
  }

  // Submit button for saving all changes.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 100,
  );

  // Return the form.
  return $form;
}

/**
 * Forest plan inventory form validation.
 */
function farm_forest_plan_inventory_form_validate($form, &$form_state) {

}

/**
 * Forest plan inventory form submit.
 */
function farm_forest_plan_inventory_form_submit($form, &$form_state) {

  // Get the plan.
  $plan = $form_state['values']['plan'];

  // Get all log values.
  $log_values = $form_state['values']['logs'];

  // Create an array to add logs to.
  $logs = array();

  // Iterate through the log values.
  foreach ($log_values as $log_id => $values) {

    // Add the silviculture score to notes. This will only be set on new logs.
    if (!empty($values['score'])) {
      $notes = '';
      if (!empty($values['notes']['value'])) {
        $notes .= "\n\n" . $values['notes']['value'];
      }
      $values['notes']['value'] = t('Silviculture score') . ': ' . $values['score'] . $notes;
    }

    // Pass values to the log details form submit function.
    $log = farm_forest_plan_log_details_form_submit($plan, $log_id, $values, 'volume');

    // If no log was created, skip it.
    if (empty($log)) {
      continue;
    }

    // Define measurement defaults.
    $measurement_defaults = array(
      'area' => array(
        'measure' => 'area',
        'value' => '',
        'units' => $form_state['values']['units']['area'],
        'label' => 'total area',
      ),
      'volume' => array(
        'measure' => 'volume',
        'value' => '',
        'units' => $form_state['values']['units']['volume'],
        'label' => 'total volume',
      ),
      'height' => array(
        'measure' => 'length',
        'value' => '',
        'units' => $form_state['values']['units']['length'],
        'label' => 'maximum height',
      ),
      'mai' => array(
        'measure' => 'length',
        'value' => '',
        'units' => $form_state['values']['units']['length'],
        'label' => 'mean annual increment',
      ),
    );

    // Add/update measurements...

    // Generate an array of measurements from the form values.
    $measurements = array();
    foreach ($measurement_defaults as $key => $default) {
      $measurement = $measurement_defaults[$key];
      $measurement['value'] = $values[$key];
      $measurements[$key] = $measurement;
    }

    // Load a list of existing measurement values from the log.
    $old_values = farm_forest_plan_inventory_log_measurements($log);

    // Iterate through the measurements.
    foreach ($measurements as $key => $measurement) {

      // If the measurement has a value...
      if (!empty($measurement['value'])) {

        // If this is a new log, or if the old value was never set, add a new
        // measurement.
        if ($log_id == 'new' || $old_values[$key] === '') {
          farm_quantity_log_add_measurements($log, array($measurement));
        }

        // Otherwise, attempt to update an existing measurement value by label.
        else {
          farm_quantity_log_update_measurement_by_label($log, $measurement['label'], $measurement['value']);
        }
      }

      // If the measurement does not have a value, but it did previously,
      // delete it.
      elseif (!empty($old_values[$key])) {
        farm_quantity_log_delete_measurement_by_label($log, $measurement['label']);
      }
    }

    // If a log is available, add it to the array.
    if (!empty($log)) {
      $logs[] = $log;
    }
  }

  // Add logs to the form state storage so future submit handlers can use them.
  $form_state['storage']['logs'] = $logs;
}

/**
 * Helper function for loading existing measurement values from an inventory log.
 *
 * @param $log
 *   The inventory log.
 *
 * @return array
 *   Returns an array of values keyed by form element ID.
 */
function farm_forest_plan_inventory_log_measurements($log) {
  $values = array(
    'area' => '',
    'volume' => '',
    'height' => '',
    'mai' => '',
  );
  if (empty($log)) {
    return $values;
  }
  $measurements = farm_quantity_log_data($log);
  foreach ($measurements as $measurement) {
    if ($measurement['label'] == 'total area') {
      $values['area'] = $measurement['value'];
    }
    elseif ($measurement['label'] == 'total volume') {
      $values['volume'] = $measurement['value'];
    }
    elseif ($measurement['label'] == 'maximum height') {
      $values['height'] = $measurement['value'];
    }
    elseif ($measurement['label'] == 'mean annual increment') {
      $values['mai'] = $measurement['value'];
    }
  }
  return $values;
}
