<?php
// $Id: FeedsSyndicationParser.inc,v 1.5 2009/10/28 01:12:47 alexb Exp $

/**
 * Class definition for Common Syndication Parser.
 *
 * Parses RSS and Atom feeds.
 */
class FeedsSyndicationParser extends FeedsParser {

  /**
   * Parses a raw string and returns a Feed object from it.
   */
  public function parse(FeedsFetcherResult $fetcherResult, FeedsSource $source) {
    if ($fetcherResult->type == 'text/filepath') {
      $string = file_get_contents($fetcherResult->value);
    }
    else {
      $string = $fetcherResult->value;
    }
    feeds_include_library('common_syndication_parser.inc', 'common_syndication_parser');
    return new FeedsParserResult(common_syndication_parser_parse($string), 'syndication');
  }

  /**
   * Return mapping sources.
   *
   * At a future point, we could expose data type information here,
   * storage systems like Data module could use this information to store
   * parsed data automatically in fields with a correct field type.
   */
  public function getMappingSources() {
    return array(
      'title' => array(
        'name' => t('Title'),
        'description' => t('The title of the feed item.'),
      ),
      'description' => array(
        'name' => t('Description'),
        'description' => t('Description of the feed item.'),
      ),
      'author' => array(
        'name' => t('Author'),
        'description' => t('Author of the feed item (string).'),
      ),
      'timestamp' => array(
        'name' => t('Published date'),
        'description' => t('The published date as UNIX time GMT of the feed item.'),
      ),
      'url' => array(
        'name' => t('Item URL (link)'),
        'description' => t('The URLl of the feed item.'),
      ),
      'guid' => array(
        'name' => t('Item GUID'),
        'description' => t('The Global Unique Identifier of the feed item.'),
      ),
      'tags' => array(
        'name' => t('Categories'),
        'description' => t('An array of categories that have been assigned to the feed item.'),
      ),
     );
  }
}