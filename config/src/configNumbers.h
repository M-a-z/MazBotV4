/* ********************************************************************
 *
 * @file config.c
 * @brief configuration number definitions.
 *
 *
 * -Revision History:
 *
 *  17.08.2009/Maz  First draft
 *
 *
 *  Lisence info: You're allowed to use / modify this - you're only required to
 *  write me a short note and your opinion about this to Mazziesaccount@gmail.com.
 *  if you redistribute this you're required to mention the original author:
 *  "Maz - http://maz-programmersdiary.blogspot.com/" in your distribution
 *  channel.
 *
 *
 *  PasteLeft 2009 Maz.
 *  *************************************************************/


#ifndef MAZ_BOT_V4_CONFIG_NUMBERS_H
#define MAZ_BOT_V4_CONFIG_NUMBERS_H

typedef enum EmbotCfg
{
	E_own_id					= 0,
	E_callback_event_file		= 1,
	E_event_def_reply_file		= 2,
	E_text_event_prefix_in_use	= 3,
	E_text_event_prefix			= 4,
	E_own_nick					= 5,
	E_user_ident_mode			= 6,
	E_user_level_file_name_reg	= 7,
	E_user_level_file_name_pass = 8,
	E_user_level_file_name_host = 9,
	E_owner_passwd				= 10,
	E_serverlist				= 11,
	E_serveramnt				= 12,
	E_serverchans				= 13,
	E_nmbr_of_configs
}EmbotCfg;

#endif
