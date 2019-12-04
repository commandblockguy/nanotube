#include <stddef.h>
#include "queue.h"

/* You know that feeling where you type a word so many times it no longer feels like a real word? */
/* Yeah. */

bool queue_add(queue_t *queue, void *elem) {
	if((queue->first == queue->last + 1 && !queue_empty(queue)) ||
	   (queue->first == 0 && queue->last == QUEUE_NUM_ELEMS - 1)) {
		/* Queue is full */
		return false;
	}

	queue->last++;
	queue->last %= QUEUE_NUM_ELEMS;
	queue->elems[queue->last] = elem;

	return true;
}

void *queue_get(queue_t *queue) {
	void *ret;
	if(queue_empty(queue)) return NULL;

	ret = queue->elems[queue->first];

	if(queue->first == queue->last) {
		queue->first = 0;
		queue->last = -1;
		return ret;
	}

	queue->first++;
	queue->first %= QUEUE_NUM_ELEMS;

	return ret;
}
