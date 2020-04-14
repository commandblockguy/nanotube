#ifndef NANOTUBE_QUEUE_H
#define NANOTUBE_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

#define QUEUE_NUM_ELEMS 16

typedef struct Queue {
	int8_t first;
	int8_t last;
	void *elems[QUEUE_NUM_ELEMS];
} queue_t;
#define QUEUE_INITIALIZER {0, -1}


/* Returns true if the add was successful */
bool queue_add(queue_t *queue, void *elem);
/* Returns NULL if the queue was empty */
void *queue_get(queue_t *queue);

void queue_init(queue_t *queue);

#define queue_empty(queue) ((queue)->last == -1)


#endif //NANOTUBE_QUEUE_H
