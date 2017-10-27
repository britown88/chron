#include "IntrusiveHeap.h"
#include "libutils\CheckedMemory.h"
#include "libutils\Defs.h"

QueueElem dijkstrasRun(Dijkstras *self){
   while (!priorityQueueIsEmpty(self->queue)){
      u32 nCount = 0, nodeSize = 0;
      QueueElem *nBegin = NULL, *nEnd = NULL;
      QueueElem current = priorityQueuePop(self->queue);      

      if (dijkstrasProcessCurrent(self, current, priorityQueueIsEmpty(self->queue))){
         return current;
      }

      nCount = dijkstrasGetNeighbors(self, current, &nBegin);
      if (nCount){
         nEnd = nBegin + nCount;
         while (nBegin != nEnd){
            QueueElem n = *nBegin++;
            if (dijkstrasProcessNeighbor(self, current, n)){
               priorityQueueDecreaseKey(self->queue, n);
            }
         }
      }

      
   }

   return NULL;
}

u32 dijkstrasGetNeighbors(Dijkstras *self, QueueElem node, QueueElem **outList){
   return self->vTable->getNeighbors(self, node, outList);
}
i32 dijkstrasProcessNeighbor(Dijkstras *self, QueueElem current, QueueElem node){
   return self->vTable->processNeighbor(self, current, node);
}
i32 dijkstrasProcessCurrent(Dijkstras *self, QueueElem node, bool last){
   return self->vTable->processCurrent(self, node, last);
}
void dijkstrasDestroy(Dijkstras *self){
   self->vTable->destroy(self);
}



