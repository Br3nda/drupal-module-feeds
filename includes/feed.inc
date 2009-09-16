<?php
// $Id: feed.inc,v 1.2 2009/09/16 20:26:06 alexb Exp $
/**
 * @file
 */

/**
 * Importer class.
 * 
 * After fetch() has been invoked, it can be cached and revoked in a subsequent 
 * page callback until it is reset().
 */
class Feed extends FeedsConfigurable {

  public $fetcher, $parser, $processors; 

  /**
   * Instantiate class variables.
   */
  public function __construct($id, $config = NULL) {
    parent::__construct($id, $config);

    feeds_require_plugin($this->config['fetcher']);
    $this->fetcher = new $this->config['fetcher']($id);

    feeds_require_plugin($this->config['parser']);
    $this->parser = new $this->config['parser']($id);

    foreach ($this->config['processors'] as $processor) {
      feeds_require_plugin($processor);
      $this->processors[$processor] = new $processor($id);
    }
  }

  /**
   * Return defaults for feed configuration.
   */
  public function getDefaultConfig() {
    return array(
      'fetcher' => 'FeedsHttpFetcher',
      'parser' => 'FeedsSyndicationParser',
      'processors' => array('FeedsNodeProcessor'),
      'content_type' => '',
      'source' => array(),
      'update' => 0,
      'refresh_period' => FEEDAPI_CRON_NEVER_REFRESH,
    );
  }

  /**
   * Import feed by using configured fetchers, parsers, processors.
   */
  public function import() {
    $raw = $this->fetcher->fetch($this->source);
    $parsed = $this->parser->parse($raw);
    foreach ($this->processors as $processor) {
      $processor->process($parsed);
    }
  }

  /**
   * Set active fetcher. Does not save fetcher to configuration.
   *
   * @param $fetcher
   *   String that is the class name of the fetcher.
   */
  public function setFetcher($fetcher) {
    unset($this->fetcher);
    feeds_require_plugin($fetcher);
    $this->config['fetcher'] = $fetcher;
    $this->fetcher = new $fetcher($this->id);
  }

  /**
   * Set active parser. Does not save parser to configuration.
   *
   * @param $parser
   *   String that is the class name of the parser.
   */
  public function setParser($parser) {
    unset($this->parser);
    feeds_require_plugin($parser);
    $this->config['parser'] = $parser;
    $this->save();
    $this->parser = new $parser($this->id);
  }

  /**
   * Set active processors.
   *
   * @param $processors
   *  Array of strings that are the processors of this feed.
   */
  public function setProcessors($processors) {
    unset($this->processors);
    $this->processors = array();
    $this->config['processors'] = $processors;
    $this->save();
    foreach ($processors as $processor) {
      feeds_require_plugin($processor);
      $this->processors[] = new $processor($this->id);
    }
  }
}

/**
 * Base class for configurable, peristent objects.
 */
class FeedsConfigurable {
  protected $config;
  protected $id;

  /**
   * Constructor.
   *
   * @param $id
   *   String identifier of this object.
   * @param $config
   *   Configuration of this object. If not available, will attempt to load from database.
   */
  public function __construct($id, $config = NULL) {
    $this->id = $id;
    if (empty($config)) {
      $this->load();
    }
    else {
      $this->config = $config;
    }
  }

  /**
   * Save configuration.
   */
  public function save() {
    $save = new stdClass();
    $save->id = $this->id;
    $save->class = get_class($this);
    $save->config = $this->config;
    db_query('DELETE FROM {feeds_configuration} WHERE id = "%s"', $save->id);
    drupal_write_record('feeds_configuration', $save);
  }

  /**
   * Load configuration and unpack.
   */
  public function load() {
    ctools_include('export');
    if ($config = ctools_export_load_object('feeds_configuration', 'conditions', array('id' => $this->id, 'class' => get_class($this)))) {
      $config = array_shift($config);
      $this->config = $config->config;
      return TRUE;
    }
    return FALSE;
  }

  public function getConfig($fallback = TRUE) {
    if ($fallback) {
      return empty($this->config) ? $this->getDefaultConfig() : $this->config;
    }
    return $this->config;
  }

  public function setConfig($config) {
    $default_keys = $this->getDefaultConfig();
    $this->config = array_intersect_key($config, $default_keys);
  }

  public function addConfig($config) {
    $this->config = array_merge($this->config, $config);
    $default_keys = $this->getDefaultConfig();
    $this->config = array_intersect_key($this->config, $default_keys);
  }

  public function getDefaultConfig() {
    return array();
  }

  public function getId() {
    return $this->id;
  }

  public function form() {
    return array();
  }
}

/**
 * Abstract class, defines interface for fetchers.
 * 
 * Not using interfaces because we need a simple inheritence tree for determining the 
 * plugin type. See hook_feeds_plugin().
 */
class FeedsFetcher extends FeedsConfigurable {
  public function fetch($source) {
    return NULL;
  }
}

/**
 * Abstract class, defines interface for parsers.
 */
class FeedsParser extends FeedsConfigurable {
  public function parse($raw) {
    return NULL;
  }
  public function getMappingSources() {
    return NULL;
  }
}

/**
 * Abstract class, defines interface for processors.
 */
class FeedsProcessor extends FeedsConfigurable {
  public function process($feed) {
    return NULL;
  }
  public function getMappingTargets() {
    return NULL;
  }

  /**
   * Build the mapping form.
   */
  public function mappingForm() {
    return array();
  }

  /**
   * Return TRUE if processor requires that the feed configuration be attached to a feed node.
   */
  public function requiresFeedAsNode() {
    return TRUE;
  }
}