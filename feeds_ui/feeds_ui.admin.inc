<?php
// $Id: feeds_ui.admin.inc,v 1.1 2009/09/16 17:58:12 alexb Exp $
/**
 * @file
 */

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
      '#description' => t('A unique identifier for this preset.'),
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
    );
  }
  else {
    $form += _feeds_ui_config_form();
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Create'),
    );
  }
  return $form;
}

/**
 * Submit handler for feeds_build_create_form().
 */
function feeds_ui_build_create_form_submit($form, &$form_state) {
  if ($form_state['values']['id']) {
    $form_state['storage']['id'] = $form_state['values']['id'];
  }
  else {
    // Build and save feed.
    feeds_include('feed');
    $feed = new Feed($form_state['storage']['id'], $form_state['values']);
    $feed->save();
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
  $form = _feeds_ui_config_form ($feed);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Edit an existing configuration.
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
 * Edit an existing configuration.
 */
function feeds_ui_build_advanced_form(&$form_state, $feed) {
  $form = array();
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Helper, build edit UI for a feed. Used for editing and creating.
 */
function _feeds_ui_config_form($feed = NULL) {
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
    '#description' => t('Select a fetcher for this preset'),
    '#default_value' => empty($feed) ? key($plugins['fetcher']) : get_class($feed->fetcher),
  );
  $form['plugins']['parser'] = array(
    '#type' => 'radios',
    '#title' => t('Parser'),
    '#options' => $plugins['parser'],
    '#description' => t('Select a parser for this preset'),
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
    '#description' => t('Select processors for this preset'),
    '#default_value' => $default,
  );
  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Basic settings'),
  );
  $form['settings']['content_type'] = array(
    '#type' => 'select',
    '#title' => t('Attach to content type'),
    '#description' => t('When you attach a feeds configuration to a node, you can use nodes for creating feeds on your site.'),
    '#options' => array('' => t('None')) + node_get_types('names'),
    '#default_value' => '',
  );
  $form['settings']['update'] = array(
    '#type' => 'radios',
    '#title' => t('Update existing'),
    '#options' => array(1 => t('Yes'), 0 => t('No')),
    '#default_value' => 0, // @todo
  );
  $period = drupal_map_assoc(array(1, 900, 1800, 3600, 10800, 21600, 32400, 43200, 86400, 172800, 259200, 604800, 1209600, 2419200, 3628800, 4838400, 7257600, 15724800, 31536000), 'format_interval');
  $period[FEEDAPI_CRON_NEVER_REFRESH] = t('Never refresh');
  $period[1] = t('As often as possible');
  $form['settings']['refresh_period'] = array(
    '#type' => 'select',
    '#title' => t('Minimum refresh period'),
    '#options' => $period,
    '#default_value' => 0, // @todo
  );
  return $form;
}