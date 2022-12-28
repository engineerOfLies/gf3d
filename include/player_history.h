#ifndef __PLAYER_HISTORY_H__
#define __PLAYER_HISTORY_H__

/**
 * @brief add an event for the player history
 * @param history the history to add to
 * @param event the name of the event to add to (if the event doesn't exist, it will be added)
 * @param key the key to the specific aspect of the event to set
 * @note conflicting keys will be replaced
 * @param value the value of the specific event to set
 * @example player_history_set_event("player","respawnPoint","level","mushroom_farm");
 * @example player_history_set_event("hive2","bossfight","state","completed");
 * @example player_history_set_event("village","margul","state","rescued");
 */
void player_history_set_event(
    const char *event,
    const char *key,
    const char *value);

/**
 * @brief check the status of a persisten history event
 * @param history the history to check
 * @param event the name of the event to check
 * @param key the key to the specific aspect of the event to set
 * @return NULL on not found or the value of the key as a string pointer.
 * @note do not free the string.  if you need the data, make a copy
 */
const char *player_history_get_event(
    const char *event,
    const char *key);

#endif
