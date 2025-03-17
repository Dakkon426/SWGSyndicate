-- Add Discord support to accounts table
ALTER TABLE `swgemu`.`accounts` 
ADD COLUMN `discord_user_id` bigint(20) unsigned DEFAULT NULL AFTER `salt`,
ADD UNIQUE KEY `idx_discord_user_id` (`discord_user_id`); 