<?php
// $Id: feeds.api.php,v 1.7 2010/02/23 23:26:50 alexb Exp $

/**
 * @file
 * Doxygen API documentation for hooks invoked by Feeds.
 */

/**
 * @defgroup pluginapi Plugin API hooks
 * @{
 */

/**
 * Example of a CTools plugin hook that needs to be implemented to make
 * hook_feeds_plugins() discoverable by CTools and Feeds. The hook specifies
 * that the hook_feeds_plugins() returns Feeds Plugin API version 1 style
 * plugins.
 */
function hook_ctools_plugin_api($owner, $api) {
  if ($owner == 'feeds' && $api == 'plugins') {
    return array('version' => 1);
  }
}

/**
 * A hook_feeds_plugins() declares available Fetcher, Parser or Processor
 * plugins to Feeds. For an example look at feeds_feeds_plugin().
 *
 * @see feeds_feeds_plugin()
 */
function hook_feeds_plugins() {
  $info = array();
  $info['MyFetcher'] = array(
    'name' => 'My Fetcher',
    'description' => 'Fetches my stuff.',
    'help' => 'More verbose description here. Will be displayed on fetcher selection menu.',
    'handler' => array(
      'parent' => 'FeedsFetcher',
      'class' => 'MyFetcher',
      'file' => 'MyFetcher.inc',
      'path' => drupal_get_path('module', 'my_module'), // Feeds will look for MyFetcher.inc in the my_module directory.
    ),
  );
  $info['MyParser'] = array(
    'name' => 'ODK parser',
    'description' => 'Parse my stuff.',
    'help' => 'More verbose description here. Will be displayed on parser selection menu.',
    'handler' => array(
      'parent' => 'FeedsParser', // Being directly or indirectly an extension of FeedsParser makes a plugin a parser plugin.
      'class' => 'MyParser',
      'file' => 'MyParser.inc',
      'path' => drupal_get_path('module', 'my_module'),
    ),
  );
  $info['MyProcessor'] = array(
    'name' => 'ODK parser',
    'description' => 'Process my stuff.',
    'help' => 'More verbose description here. Will be displayed on processor selection menu.',
    'handler' => array(
      'parent' => 'FeedsProcessor',
      'class' => 'MyProcessor',
      'file' => 'MyProcessor.inc',
      'path' => drupal_get_path('module', 'my_module'),
    ),
  );
  return $info;
}

/**
 * @} End of "defgroup pluginapi".
 */

/**
 * @defgroup import Import hooks
 * @{
 */

/**
 * Invoked after a feed source has been imported.
 *
 * @param $importer
 *   FeedsImporter object that has been used for importing the feed.
 * @param $source
 *  FeedsSource object that describes the source that has been imported.
 */
function hook_feeds_after_import(FeedsImporter $importer, FeedsSource $source) {
  // See geotaxonomy module's implementation for an example.
}

/**
 * @} End of "defgroup import".
 */

/**
 * @defgroup mappingapi Mapping API hooks
 * @{
 */

/**
 * Alter mapping targets for nodes. Use this hook to add additional target
 * options to the mapping form of Node processors.
 *
 * For an example implementation, see mappers/content.inc
 *
 * @param &$targets
 *   Array containing the targets to be offered to the user. Add to this array
 *   to expose additional options. Remove from this array to suppress options.
 *   Remove with caution.
 * @param $content_type
 *   The content type of the target node.
 */
function hook_feeds_node_processor_targets_alter(&$targets, $content_type) {
  $targets['my_node_field'] = array(
    'name' => t('My custom node field'),
    'description' => t('Description of what my custom node field does.'),
    'callback' => 'my_callback',
  );
}

/**
 * Alter mapping targets for taxonomy terms. Use this hook to add additional
 * target options to the mapping form of Taxonomy term processor.
 *
 * For an example implementation, look at geotaxnomy module.
 * http://drupal.org/project/geotaxonomy
 *
 * @param &$targets
 *   Array containing the targets to be offered to the user. Add to this array
 *   to expose additional options. Remove from this array to suppress options.
 *   Remove with caution.
 * @param $vid
 *   The vocabulary id
 */
function hook_feeds_term_processor_targets_alter(&$targets, $vid) {
  if (variable_get('mymodule_vocabulary_'. $vid, 0)) {
    $targets['lat'] = array(
      'name' => t('Latitude'),
      'description' => t('Latitude of the term.'),
    );
    $targets['lon'] = array(
      'name' => t('Longitude'),
      'description' => t('Longitude of the term.'),
    );
  }
}

/**
 * Alter mapping targets for Data table entries. Use this hook to add additional
 * target options to the mapping form of Data processor.
 */
function hook_feeds_data_processor_targets_alter(&$fields, $data_table) {
  if ($data_table == mymodule_base_table()) {
    $fields['mytable:category'] = array(
      'name' => t('Category'),
      'description' => t('One or more category terms.'),
    );
  }
}

/**
 * @} End of "defgroup mappingapi".
 */
