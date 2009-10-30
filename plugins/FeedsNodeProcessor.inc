<?php
// $Id: FeedsNodeProcessor.inc,v 1.11 2009/10/30 16:55:48 alexb Exp $

/**
 * @file
 * Class definition of FeedsNodeProcessor.
 */

/**
 * Creates nodes from feed items.
 */
class FeedsNodeProcessor extends FeedsProcessor {

  /**
   * Implementation of FeedsProcessor::process().
   */
  public function process(FeedsParserResult $parserResult, FeedsSource $source) {

    // Count number of created and updated nodes.
    $created  = $updated = 0;

    foreach ($parserResult->value['items'] as $item) {
      // If the target item does not exist OR if update_existing is enabled,
      // map and save.
      if (!($nid = $this->existingItemId($item, $source)) || $this->config['update_existing']) {

        // Map item to a node.
        $node = $this->map($item);

        // Add some default information.
        $node->feeds_node_item->id = $this->id;
        $node->feeds_node_item->imported = FEEDS_REQUEST_TIME;
        $node->feeds_node_item->feed_nid = $source->feed_nid;
        if (!empty($nid)) {
          $node->nid = $nid;
        }

        // Save the node.
        node_save($node);

        if ($nid) {
          $updated++;
        }
        else {
          $created++;
        }
      }
    }

    // Set messages.
    if ($created) {
      drupal_set_message(t('Created !number !type nodes.', array('!number' => $created, '!type' => node_get_types('name', $this->config['content_type']))));
    }
    elseif ($updated) {
      drupal_set_message(t('Updated !number !type nodes.', array('!number' => $updated, '!type' => node_get_types('name', $this->config['content_type']))));
    }
    else {
      drupal_set_message(t('There is no new content.'));
    }
  }

  /**
   * Implementation of FeedsProcessor::clear().
   * @todo: use batch API.
   * @todo: node_delete() fails when user does not have appropriate permissions.
   *   http://drupal.org/node/619110
   */
  public function clear(FeedsSource $source) {
    // Count number of deleted nodes.
    $deleted = 0;

    $result = db_query('SELECT nid FROM {feeds_node_item} WHERE feed_nid = %d', $source->feed_nid);
    while ($node = db_fetch_object($result)) {
      node_delete($node->nid);
      $deleted++;
    }

    // Set message.
    if ($deleted) {
      drupal_set_message(t('Deleted !number nodes.', array('!number' => $deleted)));
    }
    else {
      drupal_set_message(t('There is no content to be deleted.'));
    }
  }

  /**
   * Implement expire().
   */
  public function expire($time = NULL) {
    if ($time === NULL) {
      $time = $this->expiryTime();
    }
    if ($time == FEEDS_EXPIRE_NEVER) {
      return;
    }
    // @todo: expires 50 at a time at the moment.
    // Create a way of letting the caller know whether all nodes could be
    // deleted. Has to be thought through in a larger context of batch
    // processing support for import and expiry.
    $result = db_query('SELECT n.nid FROM {node} n JOIN {feeds_node_item} fni ON n.nid = fni.nid WHERE fni.id = "%s" AND n.created < %d', $this->id, FEEDS_REQUEST_TIME - $time, 0, 50);
    while ($node = db_fetch_object($result)) {
      node_delete($node->nid);
    }
  }

  /**
   * Execute mapping on an item.
   */
  protected function map($source_item) {
    // Prepare node object.
    static $included;
    if (!$included) {
      module_load_include('inc', 'node', 'node.pages');
      $included = TRUE;
    }
    $target_node = new stdClass();
    $target_node->type = $this->config['content_type'];
    $target_node->feeds_node_item = new stdClass();
    node_object_prepare($target_node);
    // Assign an aggregated node always to anonymous.
    // @todo: change to feed node uid to keep in line with feedapi.
    $target_node->uid = 0;

    // Load all mappers. parent::map() might invoke their callbacks.
    self::loadMappers();

    // Have parent class do the iterating.
    return parent::map($source_item, $target_node);
  }

  /**
   * Return expiry time.
   */
  public function expiryTime() {
    return $this->config['expire'];
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    $types = node_get_types('names');
    $type = isset($types['story']) ? 'story' : key($types);
    return array(
      'content_type' => $type, // @todo: provide default content type feed_item.
      'update_existing' => 0,
      'expire' => FEEDS_EXPIRE_NEVER,
      'mappings' => array(
        '0' => array(
          'source' => 'title',
          'target' => 'title',
          'unique' => FALSE,
        ),
        '1' => array(
          'source' => 'description',
          'target' => 'body',
          'unique' => FALSE,
        ),
        '2' => array(
          'source' => 'timestamp',
          'target' => 'created',
          'unique' => FALSE,
        ),
        '3' => array(
          'source' => 'url',
          'target' => 'url',
          'unique' => TRUE,
        ),
        '4' => array(
          'source' => 'guid',
          'target' => 'guid',
          'unique' => TRUE,
        ),
      ),
    );
  }

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $types = node_get_types('names');
    $form = array();
    $form['content_type'] = array(
      '#type' => 'select',
      '#title' => t('Content type'),
      '#description' => t('Choose node type to create from this feed. <strong>Note:</strong> Users with "import !feed_id feeds" permissions will be able to <strong>import</strong> nodes of the content type selected here regardless of the node level permissions. However, users with "clear !feed_id permissions" need to have sufficient node level permissions to delete the imported nodes.', array('!feed_id' => $this->id)),
      '#options' => $types,
      '#default_value' => $this->config['content_type'],
    );
    $form['update_existing'] = array(
      '#type' => 'checkbox',
      '#title' => t('Update existing items'),
      '#description' => t('Check if existing items should be updated from the feed.'),
      '#default_value' => $this->config['update_existing'],
    );
    $period = drupal_map_assoc(array(FEEDS_EXPIRE_NEVER, 3600, 10800, 21600, 43200, 86400, 259200, 604800, 604800 * 4, 604800 * 12, 604800 * 24, 31536000), 'feeds_format_expire');
    $form['expire'] = array(
      '#type' => 'select',
      '#title' => t('Expire nodes'),
      '#options' => $period,
      '#description' => t('Select after how much time nodes should be deleted. The node\'s published date will be used for determining the node\'s age, see Mapping settings.'),
      '#default_value' => $this->config['expire'],
    );
    return $form;
  }

  /**
   * Override setTargetElement to operate on a target item that is a node.
   */
  public function setTargetElement($target_node, $target_element, $value) {
    if (in_array($target_element, array('url', 'guid'))) {
      $target_node->feeds_node_item->$target_element = $value;
    }
    elseif ($target_element == 'body') {
      $target_node->teaser = $value;
      $target_node->body = $value;
    }
    elseif (in_array($target_element, array('title', 'status', 'created'))) {
      $target_node->$target_element = $value;
    }
  }

  /**
   * Return available mapping targets.
   *
   * Static cached, may be called multiple times in a page load.
   */
  public function getMappingTargets() {
    $targets = array(
      'title' => array(
        'name' => t('Title'),
        'description' => t('The title of the node.'),
       ),
      'status' => array(
        'name' => t('Published status'),
        'description' => t('Whether a node is published or not. 1 stands for published, 0 for not published.'),
      ),
      'created' => array(
        'name' => t('Published date'),
        'description' => t('The UNIX time when a node has been published.'),
      ),
      // Using 'teaser' instead of 'body' forces entire content above the break.
      'body' => array(
        'name' => t('Body'),
        'description' => t('The body of the node. The teaser will be the same as the entire body.'),
      ),
      'url' => array(
        'name' => t('URL'),
        'description' => t('The external URL of the node. E. g. the feed item URL in the case of a syndication feed. May be unique.'),
        'optional_unique' => TRUE,
      ),
      'guid' => array(
        'name' => t('GUID'),
        'description' => t('The external GUID of the node. E. g. the feed item GUID in the case of a syndication feed. May be unique.'),
        'optional_unique' => TRUE,
      ),
    );

    self::loadMappers();
    drupal_alter('feeds_node_processor_targets', $targets, $this->config['content_type']);

    return $targets;
  }

  /**
   * Get nid of an existing feed item node if available.
   */
  protected function existingItemId($source_item, FeedsSource $source) {

    // Iterate through all unique targets and test whether they do already
    // exist in the database.
    foreach ($this->uniqueTargets($source_item) as $target => $value) {
      switch ($target) {
        case 'url':
          $nid = db_result(db_query('SELECT nid FROM {feeds_node_item} WHERE feed_nid = %d AND url = "%s"', $source->feed_nid, $value));
          break;
        case 'guid':
          $nid = db_result(db_query('SELECT nid FROM {feeds_node_item} WHERE feed_nid = %d AND guid = "%s"', $source->feed_nid, $value));
          break;
      }
      if ($nid) {
        // Return with the first nid found.
        return $nid;
      }
    }
    return 0;
  }

  /**
   * Loads on-behalf implementations from mappers/
   */
  protected static function loadMappers() {
    static $loaded = FALSE;

    if (!$loaded) {
      $path = drupal_get_path('module', 'feeds') .'/mappers';
      $files = drupal_system_listing('.*\.inc$', $path, 'name', 0);
      foreach ($files as $file) {
        if (strstr($file->filename, '/mappers/')) {
  	      require_once("./$file->filename");
        }
      }
      // Rebuild cache.
      module_implements('', FALSE, TRUE);
    }
    $loaded = TRUE;
  }
}