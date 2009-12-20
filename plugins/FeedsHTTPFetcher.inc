<?php
// $Id: FeedsHTTPFetcher.inc,v 1.7 2009/12/20 23:48:38 alexb Exp $

/**
 * @file
 * Home of the FeedsHTTPFetcher.
 */

/**
 * Fetches data via HTTP.
 */
class FeedsHTTPFetcher extends FeedsFetcher {

  /**
   * Fetch a resource via http.
   *
   * @param $resource
   *   A resource description of type FeedsResource.
   *
   * @return
   *   A string from the requested location if successful, or FALSE if not.
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    return new FeedsImportBatch($source_config['source']);
  }

  /**
   * Clear caches.
   */
  public function clear(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);
    $url = $source_config['source'];
    feeds_include_library('http_request.inc', 'http_request');
    http_request_clear_cache($url);
  }

  /**
   * Expose source form.
   */
  public function sourceForm($source_config) {
    $form = array();
    $form['source'] = array(
      '#type' => 'textfield',
      '#title' => t('URL'),
      '#description' => t('Enter a feed URL.'),
      '#default_value' => isset($source_config['source']) ? $source_config['source'] : '',
      '#maxlength' => NULL,
      '#required' => TRUE,
    );
    return $form;
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    return array('auto_detect_feeds' => FALSE);
  }
}

