<?php

/**
 * @file
 * Farm forest plan area form.
 */

/**
 * Forest plan area selection form.
 */
function farm_forest_plan_area_form($form, &$form_state, $plan) {

  // Start an empty form array.
  $form = array();

  // Set the page title.
  drupal_set_title('Forest area');

  // Add the plan to the form values, for use in form submission.
  $form['plan'] = array(
    '#type' => 'value',
    '#value' => $plan,
  );

  // Fieldset for adding an area.
  $form['add'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add area'),
  );

  // Area select.
  $form['add']['area'] = array(
    '#type' => 'textfield',
    '#title' => t('Area'),
    '#description' => t('Enter the name of an area you would like to add to this plan. Clicking on the map will automatically copy the name of the area here.'),
    '#autocomplete_path' => 'taxonomy/autocomplete/field_farm_area',
    '#required' => TRUE,
  );

  // Submit button.
  $form['add']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add area'),
    '#ajax' => array(
      'callback' => 'farm_forest_plan_area_form_ajax',
      'wrapper' => 'areas-list',
    ),
  );

  // Create a container for the areas list.
  $form['areas'] = array(
    '#prefix' => '<div id="areas-list">',
    '#suffix' => '</div>',
  );

  // Load areas that are associated with the plan.
  $area_ids = farm_plan_linked_records('area', $plan->id);

  // If there are no areas, stop now and return the form.
  if (empty($area_ids)) {
    return $form;
  }

  // Load all areas.
  $areas = taxonomy_term_load_multiple($area_ids);

  // Display a list of areas names.
  $area_list = array();
  foreach ($areas as $area) {
    $area_label = entity_label('taxonomy_term', $area);
    $area_uri = entity_uri('taxonomy_term', $area);
    $area_path = $area_uri['path'];
    $area_list[] = l($area_label, $area_path);
  }
  $form['areas']['#type'] = 'fieldset';
  $form['areas']['#title'] = t('Areas in plan');
  $form['areas']['list'] = array(
    '#type' => 'markup',
    '#markup' => theme('item_list', array('items' => $area_list)),
  );

  // Return the form.
  return $form;
}

/**
 * Form ajax function for area selection form.
 */
function farm_forest_plan_area_form_ajax($form, &$form_state) {
  return $form['areas'];
}

/**
 * Forest plan area selection form validation.
 */
function farm_forest_plan_area_form_validate($form, &$form_state) {

  // Load the area by name.
  $area_name = $form_state['values']['area'];
  $areas = taxonomy_get_term_by_name($area_name, 'farm_areas');

  // If no areas were found, throw an error.
  if (empty($areas)) {
    $message = t('No areas were found with that name.');
    form_set_error('area', $message);
  }

  // If more than one area was found, throw a warning.
  if (count($areas) > 1) {
    $message = t('Multiple areas were found with that name. Only the first one will be added.');
    drupal_set_message($message, 'warning');
  }

  // Take the first area and store it in the form state.
  $area = reset($areas);
  $form_state['storage']['area'] = $area;
}

/**
 * Forest plan area selection form submit.
 */
function farm_forest_plan_area_form_submit($form, &$form_state) {

  // If no area is stored, do nothing.
  if (empty($form_state['storage']['area']->tid)) {
    return;
  }

  // Link the area to the plan.
  farm_plan_link_record('area', $form_state['values']['plan']->id, $form_state['storage']['area']->tid);

  // Rebuild the form so that the new area shows up in the list when refreshed
  // via AJAX.
  $form_state['rebuild'] = TRUE;
}
