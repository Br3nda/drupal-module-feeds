<?php
// $Id: feeds.api.php,v 1.2 2009/10/28 00:02:21 alexb Exp $

/**
 * @file
 * Doxygen API documentation for hooks invoked by Feeds.
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
function hook_feeds_node_processor_targets_alter(&$targts, $content_type) {
  $targets['my_node_field'] = array(
    'name' => t('My custom node field'),
    'callback' => 'my_callback',
  );
}

/**
 * Alter mapping targets for taxonomy terms. Use this hook to add additional
 * target options to the mapping form of Taxonomy term processor.
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
    $targets['lat'] = t('Latitude');
    $targets['lon'] = t('Longitude');
  }
}
