<?php
// $Id: FeedsTermProcessor.inc,v 1.25.2.8 2010/10/05 22:26:58 alexb Exp $

/**
 * @file
 * FeedsTermProcessor class.
 */

/**
 * Feeds processor plugin. Create taxonomy terms from feed items.
 */
class FeedsTermProcessor extends FeedsProcessor {
  /**
   * Define entity type.
   */
  protected function __construct($id) {
    parent::__construct($id);
    $this->entity_type = 'taxonomy_term';
  }

  /**
   * Implements FeedsProcessor::process().
   */
  public function process(FeedsSource $source, FeedsParserResult $parser_result) {
    $state = $source->state(FEEDS_PROCESS);

    while ($item = $parser_result->shiftItem()) {
      if (!($tid = $this->existingItemId($source, $parser_result)) ||
            $this->config['update_existing'] != FEEDS_SKIP_EXISTING) {

        // Map item to a term.
        if ($tid && $this->config['update_existing'] == FEEDS_UPDATE_EXISTING) {
          $term = $this->loadTerm($source, $tid);
        }
        else {
          $term = $this->newTerm($source);
        }
        $term = $this->map($source, $parser_result, $term);

        // Save the term.
        $term->feeds_importer_id = $this->id;
        $term->feed_nid = $source->feed_nid;
        taxonomy_term_save($term);
        if ($tid) {
          $state->updated++;
        }
        else {
          $state->created++;
        }
      }
    }

    // Set messages when done.
    if ($source->progressImporting() == FEEDS_BATCH_COMPLETE) {
      $vocabulary = $this->vocabulary();
      if ($state->created) {
        drupal_set_message(format_plural($state->created, 'Created @number term in !vocabulary.', 'Created @number terms in !vocabulary.', array('@number' => $state->created, '!vocabulary' => $vocabulary->name)));
      }
      elseif ($state->updated) {
        drupal_set_message(format_plural($state->updated, 'Updated @number term in !vocabulary.', 'Updated @number terms in !vocabulary.', array('@number' => $state->updated, '!vocabulary' => $vocabulary->name)));
      }
      else {
        drupal_set_message(t('There are no new terms.'));
      }
    }
  }

  /**
   * Implements FeedsProcessor::clear().
   */
  public function clear(FeedsSource $source) {
    $state = $source->state(FEEDS_PROCESS_CLEAR);
    $vocabulary = $this->vocabulary();

    // Build base select statement.
    $select = db_select('taxonomy_term_data', 'td');
    $select->addField('td', 'tid');
    $select->join('feeds_item', 'fi', "td.tid = fi.entity_id AND fi.entity_type = 'taxonomy_term'");
    $select->condition('fi.id', $this->id);
    $select->condition('fi.feed_nid', $source->feed_nid);

    // If there is no total, query it.
    if (!$state->total) {
      $state->total = $select->countQuery()
        ->execute()
        ->fetchField();
    }

    // Delete a batch of items.
    $terms = $select->range(0, $this->getLimit())->execute();
    $deleted = 0;
    foreach ($terms as $term) {
      if (taxonomy_term_delete($term->tid) == SAVED_DELETED) {
        $deleted++;
      }
    }

    // Report progress, take into account that we may not have deleted as
    // many items as we have counted at first.
    if ($deleted) {
      $state->deleted += $deleted;
      $state->progress($state->total, $state->deleted);
    }
    else {
      $state->progress($state->total, $state->total);
    }

    // Report results when done.
    if ($source->progressClearing() == FEEDS_BATCH_COMPLETE) {
      if ($state->deleted) {
        drupal_set_message(format_plural($state->deleted, 'Deleted @number term from !vocabulary.', 'Deleted @number terms from !vocabulary.', array('@number' => $state->deleted, '!vocabulary' => $vocabulary->name)));
      }
      else {
        drupal_set_message(t('No terms to be deleted.'));
      }
    }
  }

  /**
   * Report processing and clearing limit.
   */
  public function getLimit() {
    return variable_get('feeds_process_limit', FEEDS_PROCESS_LIMIT);
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    return array(
      'vocabulary' => 0,
      'update_existing' => FEEDS_SKIP_EXISTING,
      'mappings' => array(),
    );
  }

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $options = array(0 => t('Select a vocabulary'));
    foreach (taxonomy_get_vocabularies() as $vocab) {
      $options[$vocab->machine_name] = check_plain($vocab->name);
    }
    $form = array();
    $form['vocabulary'] = array(
      '#type' => 'select',
      '#title' => t('Import to vocabulary'),
      '#description' => t('Choose the vocabulary to import into. <strong>CAUTION:</strong> when deleting terms through the "Delete items" tab, Feeds will delete <em>all</em> terms from this vocabulary.'),
      '#options' => $options,
      '#default_value' => $this->config['vocabulary'],
    );
    $form['update_existing'] = array(
      '#type' => 'radios',
      '#title' => t('Update existing terms'),
      '#description' => t('Select how existing terms should be updated. Existing terms will be determined using mappings that are a "unique target".'),
      '#options' => array(
        FEEDS_SKIP_EXISTING => 'Do not update existing terms',
        FEEDS_REPLACE_EXISTING => 'Replace existing terms',
        FEEDS_UPDATE_EXISTING => 'Update existing terms (slower than replacing them)',
      ),
      '#default_value' => $this->config['update_existing'],
    );
    return $form;
  }

  /**
   * Override parent::configFormValidate().
   */
  public function configFormValidate(&$values) {
    if (empty($values['vocabulary'])) {
      form_set_error('vocabulary', t('Choose a vocabulary'));
    }
  }

  /**
   * Return available mapping targets.
   */
  public function getMappingTargets() {
    $targets = parent::getMappingTargets();
    $targets += array(
      'name' => array(
        'name' => t('Term name'),
        'description' => t('Name of the taxonomy term.'),
        'optional_unique' => TRUE,
       ),
      'description' => array(
        'name' => t('Term description'),
        'description' => t('Description of the taxonomy term.'),
       ),
    );
    // Let implementers of hook_feeds_term_processor_targets() add their targets.
    if ($vocabulary = $this->vocabulary()) {
      self::loadMappers();
      feeds_alter('feeds_processor_targets', $targets, 'taxonomy_term', $vocabulary->machine_name);
    }
    return $targets;
  }

  /**
   * Get id of an existing feed item term if available.
   */
  protected function existingItemId(FeedsSource $source, FeedsParserResult $result) {
    if ($tid = parent::existingItemId($source, $result)) {
      return $tid;
    }

    // The only possible unique target is name.
    foreach ($this->uniqueTargets($source, $result) as $target => $value) {
      if ($target == 'name') {
        $vocabulary = $this->vocabulary();
        if ($tid = db_query("SELECT tid FROM {taxonomy_term_data} WHERE name = :name AND vid = :vid", array(':name' => $value, ':vid' => $vocabulary->vid))->fetchField()) {
          return $tid;
        }
      }
    }
    return 0;
  }

  /**
   * Return vocabulary to map to.
   */
  public function vocabulary() {
    if (isset($this->config['vocabulary'])) {
      if ($vocabulary = taxonomy_vocabulary_machine_name_load($this->config['vocabulary'])) {
        return $vocabulary;
      }
    }
    throw new Exception(t('No vocabulary defined for Taxonomy Term processor.'));
  }

  /**
   * Creates a new term in memory and returns it.
   */
  protected function newTerm($source) {
    $term = new stdClass();
    $this->newItemInfo($term, $source->feed_nid);
    $vocabulary = $this->vocabulary();
    $term->vid = $vocabulary->vid;
    return $term;
  }

  /**
   * Loads an existing term.
   */
  protected function loadTerm($source, $tid) {
    $term = taxonomy_term_load($tid);
    if ($this->loadItemInfo($term)) {
      $this->newItemInfo($term, $source->feed_nid);
    }
    return $term;
  }
}
