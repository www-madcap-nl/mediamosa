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
  * The job server module.
  */


class mediamosa_job_server {

  /**
   * Retrieve a listing of the jobs.
   *
   * 1.x: vpx_jobserver_get_jobs_status (not exact port(!)).
   *
   */
  static public function search() {

    return mediamosa_db::db_select(mediamosa_job_server_db::TABLE_NAME, 'js')
      ->fields('js')
      ->fields('jsa')
      ->leftJoin(mediamosa_job_server_analyse_db::TABLE_NAME, 'jsa', strtr('js.#jobserver_job_id = jsa.#jobserver_job_id', array('#jobserver_job_id' => mediamosa_job_server_db::ID)))
      ->condition(mediamosa_job_server_db::INSTALL_ID, MEDIAMOSA_INSTALL_ID)
      ->execute();
  }
}
