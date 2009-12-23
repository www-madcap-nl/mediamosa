<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

 /**
  * @file
  * The configuration module helper functions.
  */

function mediamosa_configuration_watchdog() {
  $content = t('List of the 50 most recent log entries.');

  $levels = watchdog_severity_levels();
  $types = array('job', 'jobhandler', 'jobserver', 'page not found', 'server');
  $result = db_query("SELECT * FROM {watchdog} WHERE type IN ('". implode("', '", $types) ."') ORDER BY timestamp DESC LIMIT 50");

  $rows = array(); // fix notice
  foreach ($result as $row) {
    $rows[] = array($row->type, nl2br($row->message), $levels[$row->severity], format_date($row->timestamp, 'small'));
  }
  $content .= theme('table', array(t('Type'), t('Message'), t('Severity'), t('Date')), $rows);

  return $content;
}
