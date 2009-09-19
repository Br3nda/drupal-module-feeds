<?php
// $Id: feeds_ui.admin.inc,v 1.12 2009/09/19 16:36:55 alexb Exp $
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
    // @todo: present plugin form first.
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
 * 
 * @todo: don't allow attaching more than one configuration to one content type.
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
  $form['#feed'] = $feed;

  // Build a mapping form for each processor configured.
  $sources = $feed->parser->getMappingSources();
  foreach ($feed->processors as $class => $processor) {
    // @todo: move actual form building into processors?
    $mappings = $processor->getMappings();
    $targets = $processor->getMappingTargets();
    $form['processors'][$class] = array(
      '#type' => 'fieldset',
      '#title' => $class, // @todo: human readable title.
      '#tree' => TRUE,
    );
    $form['processors'][$class]['#mappings'] = $mappings;
    $form['processors'][$class]['#targets'] = $targets;
    $form['processors'][$class]['source'] = array(
      '#type' => 'select',
      '#options' => array('' => t('Select a source')) + drupal_map_assoc($sources),
    );
    $form['processors'][$class]['target'] = array(
      '#type' => 'select',
      '#options' => array('' => t('Select a target')) + drupal_map_assoc(array_keys($targets)),
    );
    $form['processors'][$class]['add'] = array(
      '#type' => 'submit',
      '#value' => t('Add'),
    );
  }
  return $form;
}

/**
 * Submit handler for feeds_ui_build_mapping_form().
 */
function feeds_ui_build_mapping_form_submit($form, &$form_state) {
  foreach ($form['#feed']->processors as $class => $processor) {
    $processor->addMapping($form_state['values'][$class]['source'], $form_state['values'][$class]['target']);
  }
}

/**
 * Edit plugin configuration.
 */
function feeds_ui_build_plugin_form(&$form_state, $feed) {
  $form = array();
  $form['#feed'] = $feed;
  $form['plugins'] = array(
    '#type' => 'fieldset',
    '#title' => t('Plugins'),
  );
  $form['plugins'] += $feed->pluginForm($form_state);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Validation handler for feeds_ui_build_edit_form().
 */
function feeds_ui_build_plugin_form_validate($form, &$form_state) {
  $form['#feed']->pluginFormValidate($form, &$form_state);
}

/**
 * Submit handler for feeds_ui_build_edit_form().
 */
function feeds_ui_build_plugin_form_submit($form, &$form_state) {
  $form['#feed']->pluginFormSubmit($form, &$form_state);
  drupal_set_message(t('Saved configuration'));
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

/**
 * Theme function for feeds_ui_build_mapping_form().
 */
function theme_feeds_ui_build_mapping_form($form) {
  $output = '';
  $header = array(
    t('Source'),
    t('Target'),
    t('Unique'),
    '&nbsp;',
  );

  foreach (element_children($form['processors']) as $processor) {
    $rows = array();
    if (is_array($form['processors'][$processor]['#mappings'])) {
      foreach ($form['processors'][$processor]['#mappings'] as $target => $mapping) {
        $rows[] = array(
          $source,
          $mapping['source'],
          $mapping['unique'] ? t('Yes') : t('No'),
          t('Remove'),
        ); 
      }
    }
    if (!count($rows)) {
      $rows[] = array(
        array(
          'colspan' => 4,
          'data' => t('No mappings defined.'),
        ),
      );
    }
    $rows[] = array(
      drupal_render($form['processors'][$processor]['source']),
      drupal_render($form['processors'][$processor]['target']),
      '',
      drupal_render($form['processors'][$processor]['add']),
    );
    $form['processors'][$processor]['#value'] = theme('table', $header, $rows);
  }
  $output .= drupal_render($form);
  return $output;
}