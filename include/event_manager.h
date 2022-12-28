#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__

void event_manager_event_end();
void event_manager_update();
void event_manager_handle_choice(SJson *event, int choice);

#endif
