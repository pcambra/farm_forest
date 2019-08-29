<?php

/**
 * @file
 * Farm forest plan management form.
 */

/**
 * Forest plan management form.
 */
function farm_forest_plan_management_form($form, &$form_state, $plan) {

  // Start a form array.
  $form = array(
    '#tree' => TRUE,
  );

  // Set the page title.
  drupal_set_title('Management');

  // Add the plan to the form values, for use in form submission.
  $form['plan'] = array(
    '#type' => 'value',
    '#value' => $plan,
  );

  // Define the available management log types.
  $log_types = array(
    'farm_activity' => t('Activity'),
    'farm_input' => t('Input'),
  );

  // Load a list of management logs associated with this plan.
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
    $form['logs'][$log->id] = farm_forest_plan_log_details_form($log, $log_types, $assets);
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
  $form['logs']['new'] = farm_forest_plan_log_details_form(NULL, $log_types, $assets);
  $form['logs']['new']['#title'] = t('Add a management task');
  $form['logs']['new']['#collapsed'] = $collapsed;

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
 * Forest plan management form validation.
 */
function farm_forest_plan_management_form_validate($form, &$form_state) {

}

/**
 * Forest plan management form submit.
 */
function farm_forest_plan_management_form_submit($form, &$form_state) {

  // Get the plan.
  $plan = $form_state['values']['plan'];

  // Get all log values.
  $log_values = $form_state['values']['logs'];

  // Create an array to add logs to.
  $logs = array();

  // Iterate through the log values.
  foreach ($log_values as $log_id => $values) {

    // Pass values to the log details form submit function.
    $log = farm_forest_plan_log_details_form_submit($plan, $log_id, $values);

    // If a log is available, add it to the array.
    if (!empty($log)) {
      $logs[] = $log;
    }
  }

  // Add logs to the form state storage so future submit handlers can use them.
  $form_state['storage']['logs'] = $logs;
}
