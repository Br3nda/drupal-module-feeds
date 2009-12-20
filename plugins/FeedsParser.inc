<?php
// $Id: FeedsParser.inc,v 1.2 2009/12/20 23:27:28 alexb Exp $

/**
 * Abstract class, defines interface for parsers.
 *
 * @todo: make implement FeedsSourceInterface
 */
abstract class FeedsParser extends FeedsPlugin {

  /**
   * Parse content fetched by fetcher.
   *
   * Stub method. Extending classes must implement this method.
   *
   * @param $batch
   *   FeedsImportBatch returned by fetcher.
   * @param FeedsSource $source
   *   Source information.
   * @return
   *   A parsed array.
   *   @todo: define this array (object?).
   *
   * @todo: Should it be execute() ?
   */
  public abstract function parse(FeedsImportBatch $batch, FeedsSource $source);

  /**
   * Clear all caches for results for given source.
   *
   * @param FeedsSource $source
   *   Source information for this expiry. Implementers can choose to only clear
   *   caches pertaining to this source.
   */
  public function clear(FeedsSource $source) {}

  /**
   * Declare the possible mapping sources that this parser produces.
   *
   * @return
   *   An array of mapping sources, or FALSE if the sources can be defined by
   *   typing a value in a text field.
   *
   *   Example:
   *   array(
   *     'title' => t('Title'),
   *     'created' => t('Published date'),
   *     'url' => t('Feed item URL'),
   *     'guid' => t('Feed item GUID'),
   *   )
   */
  public function getMappingSources() {
    return FALSE;
  }

  /**
   * Get an element identified by $element_key of the given item.
   * The element key corresponds to the values in the array returned by
   * FeedsParser::getMappingSources().
   */
  public function getSourceElement($item, $element_key) {
    return isset($item[$element_key]) ? $item[$element_key] : '';
  }
}