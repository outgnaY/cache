#ifndef SLAB_H
#define SLAB_H

void slabs_init();

unsigned int slabs_clsid(const int size);
unsigned int slabs_size(const int clsid);








#endif