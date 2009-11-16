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
  * Authorization layer for assets, mediafiles DB Classes.
  */

class mediamosa_aut_name_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_aut_name'; // 1.x aut_name table.

  // Primary KEY
  const ID = 'aut_name_id';

  // Database fields;
  const APP_ID = 'app_id';
  const AUT_GROUP_ID = 'aut_group_id';
  const AUT_NAME = 'aut_name';
  const AUT_NAME_LENGTH = 255;
  const AUT_PREFIX = 'aut_prefix';
  const AUT_PREFIX_LENGTH = 128;
  const AUT_TYPE = 'aut_type';
  const AUT_TYPE_DOMAIN = 'DOMAIN';
  const AUT_TYPE_REALM = 'REALM';
  const AUT_TYPE_USER = 'USER';
  const AUT_TYPE_USER_GROUP = 'USER_GROUP';
}

class mediamosa_aut_group_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_aut_group'; // 1.x aut_group table.

  // Primary KEY
  const ID = 'aut_group_id';

  // Database fields;
  const APP_ID = 'app_id';
  const AUT_GROUP_NAME = 'aut_group_name';
  const AUT_GROUP_NAME_LENGTH = 128;
  const AUT_GROUP_TYPE = 'aut_group_type';
  const AUT_GROUP_TYPE_REALM = 'REALM';
  const AUT_GROUP_TYPE_DOMAIN = 'DOMAIN';

}

class mediamosa_aut_object_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_aut_object'; // 1.x aut_object table.

  // Database fields;
  const AUT_OBJECT_ID = 'aut_object_id';
  const AUT_OBJECT_ID_LENGTH = 32;
  const AUT_OBJECT_TYPE = 'aut_object_type';
  // The object types;
  const AUT_OBJECT_TYPE_MEDIAFILE = 'MEDIAFILE';
  const AUT_OBJECT_TYPE_ASSET = 'ASSET';
  const AUT_OBJECT_TYPE_COLLECTION = 'COLLECTION';

  const AUT_ID = 'aut_id';
  const AUT_TYPE = 'aut_type';
  const AUT_TYPE_NAME = 'NAME';
  const AUT_TYPE_GROUP = 'GROUP';
}

class mediamosa_app_master_slave_db {
  // ------------------------------------------------------------------ Consts.
  // Table name;
  const TABLE_NAME = 'mediamosa_app_master_slave'; // 1.x aut_object table.

  // Database fields;
  const AUT_OBJECT_ID = 'aut_object_id';
  const AUT_OBJECT_TYPE = 'aut_object_type';
  // The object types;
  const AUT_OBJECT_TYPE_MEDIAFILE = 'MEDIAFILE';
  const AUT_OBJECT_TYPE_ASSET = 'ASSET';
  const AUT_OBJECT_TYPE_COLLECTION = 'COLLECTION';

  const APP_ID_SLAVE = 'app_id_slave'; // 1.x app_slave_id
  const APP_ID_MASTER = 'app_id_master'; // 1.x app_maser_id
}
