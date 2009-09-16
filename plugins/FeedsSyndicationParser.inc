<?php
// $Id: FeedsSyndicationParser.inc,v 1.1 2009/09/16 17:58:13 alexb Exp $

class FeedsSyndicationParser extends FeedsParser {

  public static function getName() {
    return t('Syndication Parser');
  }

  public static function getDescription() {
    return t('Parses RSS and Atom feeds.');
  }

  /**
   * Parses a raw string and returns a Feed object from it.
   */
  public function parse($raw) {
    
  }

  /**
   * Return mapping sources.
   */
  public function getMappingSources() {
    return array('title', 'description', 'url', 'guid', 'tags');
  }
}