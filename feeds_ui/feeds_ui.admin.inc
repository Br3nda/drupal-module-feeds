<?php
// $Id: feeds_ui.admin.inc,v 1.4 2009/09/16 21:41:30 alexb Exp $
/**
 * @file
 */

/**
 * List available feed configurations.
 */
function feeds_ui_content_overview() {
  $rows = array();
  if ($feeds = feeds_load_all()) {
    foreach ($feeds as $feed) {
      $rows[] = array(
        $feed->getId(),
      );
    }
  }
  $header = array(
    t('Configurations'),
  );
  return theme('table', $header, $rows);
}

/**
 * Build overview of available configurations.
 */
function feeds_ui_build_overview() {
  $rows = array();
  if ($feeds = feeds_load_all()) {
    foreach ($feeds as $feed) {
      $rows[] = array(
        $feed->getId(),
        l(t('Edit'), 'admin/build/feeds/edit/'. $feed->getId()) .' | '. 
        l(t('Delete'), 'admin/build/feeds/delete/'. $feed->getId()),
      );
    }
  }
  $rows[] = array(
    l(t('New configuration'), 'admin/build/feeds/create'),
    '&nbsp;',
  );
  $header = array(
    t('Configurations'),
    t('Operations'),
  );
  return theme('table', $header, $rows);
}

/**
 * Create a new configuration.
 */
function feeds_ui_build_create_form(&$form_state) {
  $form = array();
  if (!$form_state['storage']['id']) {
    $form['id'] = array(
      '#type' => 'textfield',
      '#title' => t('Id'),
      '#description' => t('A unique identifier for this configuration.'),
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
    );
  }
  else {
    feeds_include('feed');
    $feed = feeds_get_feed($form_state['storage']['id']);
    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Basic configuration'),
    );
    $form['settings'] += $feed->configForm($form_state);
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Create'),
    );
  }
  return $form;
}

/**
 * Validation handler for feeds_build_create_form().
 */
function feeds_ui_build_create_form_validate($form, &$form_state) {
  ctools_include('export');
  if (isset($form_state['values']['id'])) {
    $feed = feeds_get_feed($form_state['values']['id']);
    if (ctools_export_load_object('feeds_configuration', 'conditions', array('id' => $form_state['values']['id'], 'class' => 'Feed'))) {
      form_set_error('id', t('Id is taken.'));
    }
  }
  elseif (isset($form_state['storage']['id'])) {
    $feed = feeds_get_feed($form_state['storage']['id']);
    if (ctools_export_load_object('feeds_configuration', 'conditions', array('id' => $form_state['storage']['id'], 'class' => 'Feed'))) {
      form_set_error('id', t('Id is taken.'));
      drupal_goto('admin/build/feeds/create');
    }
  }
  $feed->configFormValidate($form, &$form_state);
}

/**
 * Submit handler for feeds_build_create_form().
 */
function feeds_ui_build_create_form_submit($form, &$form_state) {
  if ($form_state['values']['id']) {
    $form_state['storage']['id'] = $form_state['values']['id'];
  }
  else {
    // Save feed.
    $feed = feeds_get_feed($form_state['storage']['id']);
    $feed->configFormSubmit($form, &$form_state);
    drupal_set_message(t('Created configuration'));

    // Unset storage to enable redirect.
    unset($form_state['storage']);
    $form_state['redirect'] = 'admin/build/feeds';
  }
}

/**
 * Edit an existing configuration.
 */
function feeds_ui_build_edit_form(&$form_state, $feed) {
  $form['#feed'] = $feed;
  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Basic configuration'),
  );
  $form['settings'] += $feed->configForm($form_state);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Validation handler for feeds_ui_build_edit_form().
 */
function feeds_ui_build_edit_form_validate($form, &$form_state) {
  $form['#feed']->configFormValidate($form, &$form_state);
}

/**
 * Submit handler for feeds_ui_build_edit_form().
 */
function feeds_ui_build_edit_form_submit($form, &$form_state) {
  $form['#feed']->configFormSubmit($form, &$form_state);
  drupal_set_message(t('Saved configuration'));
}

/**
 * Edit mapping.
 */
function feeds_ui_build_mapping_form(&$form_state, $feed) {
  $form = array();
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Edit plugin configuration.
 * 
 * @todo: move plugin form generation into feed class.
 */
function feeds_ui_build_plugin_form(&$form_state, $feed) {
  $form = array();
  $plugins = feeds_get_plugins();
  $form['plugins'] = array(
    '#type' => 'fieldset',
    '#title' => t('Plugins'),
  );
  $form['plugins']['fetcher'] = array(
    '#type' => 'radios',
    '#title' => t('Fetcher'),
    '#options' => $plugins['fetcher'],
    '#description' => t('Select a fetcher for this configuration.'),
    '#default_value' => empty($feed) ? key($plugins['fetcher']) : get_class($feed->fetcher),
  );
  $form['plugins']['parser'] = array(
    '#type' => 'radios',
    '#title' => t('Parser'),
    '#options' => $plugins['parser'],
    '#description' => t('Select a parser for this configuration.'),
    '#default_value' =>  empty($feed) ? key($plugins['parser']) : get_class($feed->parser),
  );
  if (empty($feed)) {
    $default = array(key($plugins['processor']));
  }
  else {
    foreach ($feed->processors as $processor) {
      $default[get_class($processor)] = get_class($processor);
    }
  }
  $form['plugins']['processors'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Processors'),
    '#options' => $plugins['processor'],
    '#description' => t('Select processors for this configuration.'),
    '#default_value' => $default,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Delete form.
 */
function feeds_ui_build_delete_form(&$form_state, $feed) {
  $form['#redirect'] = 'admin/build/feeds';
  $form['#feed'] = $feed;
  return confirm_form($form,
    t('Would you really like to delete the configuration !id?', array('!id' => $feed->getId())),
    $form['#redirect'],
    t('This action cannot be undone.'),
    t('Delete')
  );
}

/**
 * Submit handler for feeds_ui_build_delete_form().
 */
function feeds_ui_build_delete_form_submit($form, &$form_state) {
  feeds_delete($form['#feed']->getId());
}