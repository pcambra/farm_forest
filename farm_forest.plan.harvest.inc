<?php

/**
 * @file
 * Farm forest plan harvest form.
 */

/**
 * Forest plan harvest form.
 */
function farm_forest_plan_harvest_form($form, &$form_state, $plan) {

  // Start a form array.
  $form = array(
    '#tree' => TRUE,
  );

  // Set the page title.
  drupal_set_title('Harvest');

  // Add the plan to the form values, for use in form submission.
  $form['plan'] = array(
    '#type' => 'value',
    '#value' => $plan,
  );

  // Define the available harvest log types.
  $log_types = array(
    'farm_harvest' => t('Harvest'),
  );

  // Load a list of harvest logs associated with this plan.
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

  // Fieldset for adding a task.
  // If there are logs already, collapse it.
  $collapsed = !empty($logs) ? TRUE : FALSE;
  $form['logs']['new'] = farm_forest_plan_log_details_form(NULL, $log_types, $assets, FALSE);
  $form['logs']['new']['#title'] = t('Record a harvest');
  $form['logs']['new']['#collapsed'] = $collapsed;

  // Load the system of measurement and define units.
  $system_of_measurement = farm_quantity_system_of_measurement();
  $units = 'cubic meters';
  if ($system_of_measurement == 'us') {
    $units = 'cubic feet';
  }
  $form['volume_units'] = array(
    '#type' => 'value',
    '#value' => $units,
  );

  // Iterate through the log fieldsets and add harvest specific fields.
  foreach (element_children($form['logs']) as $key) {

    // Harvest type (this is added to notes, so it is only a separate field for
    // new logs).
    if ($key == 'new') {
      $type_options = array(
        'Normal harvest',
        'Commercial thinning',
        'Thinning to waste',
      );
      $form['logs'][$key]['harvest_type'] = array(
        '#type' => 'select',
        '#title' => t('Harvest type'),
        '#options' => drupal_map_assoc($type_options),
        '#weight' => 20,
      );
    }

    // Volume.
    $volume_default = '';
    if ($key != 'new') {
      $volume_measure = farm_quantity_log_data($form['logs'][$key]['log']['#value'], 'volume', t('total harvest'));
      if (!empty($volume_measure[0]['value'])) {
        $volume_default = $volume_measure[0]['value'];
      }
    }
    $form['logs'][$key]['volume'] = array(
      '#type' => 'textfield',
      '#title' => t('Volume'),
      '#input_group' => TRUE,
      '#field_suffix' => t($units),
      '#element_validate' => array('element_validate_integer_positive'),
      '#default_value' => $volume_default,
      '#weight' => 30,
    );
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
 * Forest plan harvest form validation.
 */
function farm_forest_plan_harvest_form_validate($form, &$form_state) {

}

/**
 * Forest plan harvest form submit.
 */
function farm_forest_plan_harvest_form_submit($form, &$form_state) {

  // Get the plan.
  $plan = $form_state['values']['plan'];

  // Get all log values.
  $log_values = $form_state['values']['logs'];

  // Create an array to add logs to.
  $logs = array();

  // Iterate through the log values.
  foreach ($log_values as $log_id => $values) {

    // Add the harvest type to notes. This will only be set on new logs.
    if (!empty($values['harvest_type'])) {
      $notes = '';
      if (!empty($values['notes']['value'])) {
        $notes .= "\n\n" . $values['notes']['value'];
      }
      $values['notes']['value'] = t('Harvest type') . ': ' . $values['harvest_type'] . $notes;
    }

    // Pass values to the log details form submit function.
    $log = farm_forest_plan_log_details_form_submit($plan, $log_id, $values, 'volume');

    // If no log was created, skip it.
    if (empty($log)) {
      continue;
    }

    // Save the volume measurement (add to new logs, update on existing).
    if ($log_id == 'new') {
      $measurement = array(
        'measure' => 'volume',
        'value' => $values['volume'],
        'units' => $form_state['values']['volume_units'],
        'label' => 'total harvest',
      );
      farm_quantity_log_add_measurements($log, array($measurement));
    }
    else {
      farm_quantity_log_update_measurement_by_label($log, 'total harvest', $values['volume']);
    }

    // If a log is available, add it to the array.
    if (!empty($log)) {
      $logs[] = $log;
    }
  }

  // Add logs to the form state storage so future submit handlers can use them.
  $form_state['storage']['logs'] = $logs;
}
