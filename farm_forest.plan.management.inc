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

  // Load a list of logs associated with this plan.
  $log_ids = farm_plan_linked_records('log', $plan->id);
  $logs = array();
  foreach ($log_ids as $id) {
    $log = log_load($id);
    if (!empty($log)) {
      $logs[] = $log;
    }
  }

  // Define the available management log types.
  $log_types = array(
    'farm_activity' => t('Activity'),
    'farm_observation' => t('Observation'),
    'farm_input' => t('Input'),
  );

  // Iterate through the logs and filter out non-matching types.
  $management_logs = array();
  foreach ($logs as $delta => $log) {
    if (array_key_exists($log->type, $log_types)) {
      $management_logs[] = $log;
    }
  }

  // Load a list of assets associated with this plan.
  $asset_ids = farm_plan_linked_records('asset', $plan->id);
  $assets = array();
  foreach ($asset_ids as $id) {
    $asset = farm_asset_load($id);
    if (!empty($asset)) {
      $assets[] = $asset;
    }
  }

  // Sort the logs by timestamp ascending.
  usort($management_logs, 'farm_forest_plan_management_log_sort');

  // Add fieldsets for each existing management log.
  foreach ($management_logs as $log) {
    $form['logs'][$log->id] = farm_forest_plan_management_log_details_form($log, $log_types, $assets);
    $form['logs'][$log->id]['#title'] = date('Y-m-d', $log->timestamp) . ' ' . $log_types[$log->type] . ': ' . entity_label('log', $log);

    // If the log has an ID, add a link to remove it.
    if (!empty($log->id)) {
      $form['logs'][$log->id]['remove'] = array(
        '#type' => 'markup',
        '#markup' => l(t('Remove'), 'farm/plan/' . $plan->id . '/log/' . $log->id . '/delete', array('query' => array('destination' => current_path()))),
      );
    }
  }

  // Fieldset for adding a management task.
  // If there are management logs already, collapse it.
  $collapsed = !empty($management_logs) ? TRUE : FALSE;
  $form['logs']['new'] = farm_forest_plan_management_log_details_form(NULL, $log_types, $assets);
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
 * Log timestamp ascending sort function, for use as a usort() callback.
 */
function farm_forest_plan_management_log_sort($loga, $logb) {

  // If the timestamps are the same, sort by ID (ascending).
  if ($loga->timestamp == $logb->timestamp) {
    return ($loga->id < $logb->id) ? -1 : 1;
  }

  // Otherwise, sort by timestamp (ascending).
  return ($loga->timestamp < $logb->timestamp) ? -1 : 1;
}

/**
 * Log details sub-form.
 *
 * @param $log
 *   The log entity, or NULL.
 *
 * @return array
 *   Returns a form array that can be used in another form.
 *
 * @see farm_forest_plan_management_form().
 */
function farm_forest_plan_management_log_details_form($log=NULL, $log_types, $assets) {

  // Build a fieldset for the form.
  $form = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  // Save the log to the form values.
  if (!empty($log)) {
    $form['log'] = array(
      '#type' => 'value',
      '#value' => $log,
    );
  }

  // Log type (only selectable for new logs).
  if (empty($log)) {
    $form['type'] = array(
      '#type' => 'select',
      '#title' => t('Type'),
      '#descriptions' => t('Will this task be an activity, or a passive observation?'),
      '#options' => $log_types,
      '#default_value' => 'activity',
    );
  }

  // Log name.
  $log_name = !empty($log->name) ? $log->name : '';
  $name_autocomplete_types = implode('+', array_keys($log_types));
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Task name'),
    '#autocomplete_path' => 'log/autocomplete/' . $name_autocomplete_types,
    '#default_value' => $log_name,
    '#required' => !empty($log->id) ? TRUE : 0,
  );

  // Date field.
  $date_format = 'Y-m-d';
  $log_timestamp = !empty($log->timestamp) ? $log->timestamp : date($date_format);
  $form['date'] = array(
    '#type' => 'date_select',
    '#title' => t('Date'),
    '#date_format' => $date_format,
    '#date_label_position' => 'within',
    '#date_year_range' => '-3:+10',
    '#default_value' => $log_timestamp,
    '#required' => !empty($log->id) ? TRUE : 0,
  );

  // Iterate through assets already being referenced by the log, and build an
  // array of default values.
  $selected_assets = array();
  if (!empty($log->field_farm_asset[LANGUAGE_NONE])) {
    foreach ($log->field_farm_asset[LANGUAGE_NONE] as $asset_ref) {

      // If the target ID is empty, skip it.
      if (empty($asset_ref['target_id'])) {
        continue;
      }
      $asset_id = $asset_ref['target_id'];

      // Load the asset. If it can't be loaded, skip it.
      $asset = farm_asset_load($asset_id);
      if (empty($asset)) {
        continue;
      }

      // If the asset is not in $assets, add it.
      if (!array_key_exists($asset_id, $assets)) {
        if (!empty($asset)) {
          $assets[] = $asset;
        }
      }

      // Add to the list of selected assets.
      $selected_assets[$asset_id] = $asset_id;
    }
  }

  // Build a list of asset select options.
  $asset_options = array();
  foreach ($assets as $asset) {
    $asset_options[$asset->id] = entity_label('farm_asset', $asset);
  }

  // If no selected assets were found above, and this is not an existing log,
  // select all by default.
  if (empty($selected_assets) && empty($log)) {
    $selected_assets = array_keys($asset_options);
  }

  // Select planting assets.
  if (!empty($asset_options)) {
    $form['assets'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Plantings'),
      '#descriptions' => t('Select the planting(s) that this task is associated with.'),
      '#options' => $asset_options,
      '#default_value' => $selected_assets,
    );
  }

  // Notes.
  $log_notes = !empty($log->field_farm_notes[LANGUAGE_NONE][0]['value']) ? $log->field_farm_notes[LANGUAGE_NONE][0]['value'] : '';
  $log_notes_format = !empty($log->field_farm_notes[LANGUAGE_NONE][0]['format']) ? $log->field_farm_notes[LANGUAGE_NONE][0]['format'] : 'farm_format';
  $form['notes'] = array(
    '#type' => 'text_format',
    '#title' => t('Notes'),
    '#format' => $log_notes_format,
    '#default_value' => $log_notes,
  );

  // Done.
  $log_done = !empty($log->done) ? $log->done : FALSE;
  $form['done'] = array(
    '#type' => 'checkbox',
    '#title' => t('This task is done'),
    '#default_value' => $log_done,
  );

  // Return the form array.
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
  foreach ($log_values as $key => $values) {

    // Get the log name.
    $name = $log_values[$key]['name'];

    // Get the log date as a timestamp.
    $timestamp = strtotime($log_values[$key]['date']);

    // Load selected assets.
    $assets = array();
    if (!empty($log_values[$key]['assets'])) {
      foreach ($log_values[$key]['assets'] as $asset_id) {
        $asset = farm_asset_load($asset_id);
        if (!empty($asset)) {
          $assets[] = $asset;
        }
      }
    }

    // Get areas associated with the plan.
    $area_ids = farm_plan_linked_records('area', $plan->id);
    $areas = array();
    if (!empty($area_ids)) {
      foreach ($area_ids as $area_id) {
        $area = taxonomy_term_load($area_id);
        if (!empty($area)) {
          $areas[] = $area;
        }
      }
    }

    // Load the log's "done" status.
    $done = !empty($log_values[$key]['done']) ? TRUE : FALSE;

    // If this is a new log...
    if ($key == 'new') {

      // Only proceed if a name is set.
      if (empty($name)) {
        continue;
      }

      // Get the log type.
      $type = $log_values[$key]['type'];

      // Create a new log entity.
      $log = farm_log_create($type, $name, $timestamp, $done, $assets);

      // Load a log entity metadata wrapper.
      $log_wrapper = entity_metadata_wrapper('log', $log);

      // Add notes to the log.
      if (!empty($log_values[$key]['notes']['value'])) {
        $log_wrapper->field_farm_notes = $log_values[$key]['notes'];
      }

      // Add areas to the log.
      if (!empty($areas)) {
        $log_wrapper->field_farm_area = $areas;
      }

      // Save the log again.
      $log_wrapper->save();

      // Link the log to the plan.
      farm_plan_link_record('log', $plan->id, $log->id);
    }

    // Or, if this is an existing log...
    else {

      // Get the log entity from form values.
      $log = $log_values[$key]['log'];

      // Create an entity metadata wrapper.
      $log_wrapper = entity_metadata_wrapper('log', $log);

      // Set log properties from form values.
      $log_wrapper->name->set($name);
      $log_wrapper->timestamp->set($timestamp);
      $log_wrapper->done->set($done);

      // Overwrite referenced log assets.
      $log_wrapper->field_farm_asset = $assets;

      // Overwrite reference log areas.
      $log_wrapper->field_farm_area = $areas;

      // Save the notes.
      $log_wrapper->field_farm_notes = $log_values[$key]['notes'];

      // Save the log.
      $log_wrapper->save();
    }

    // If a log is available, add it to the array.
    if (!empty($log)) {
      $logs[] = $log;
    }
  }

  // Add logs to the form state storage so future submit handlers can use them.
  $form_state['storage']['logs'] = $logs;
}
