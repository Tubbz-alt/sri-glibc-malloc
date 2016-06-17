#include <stdlib.h>
#include <sys/mman.h>

#include "lfht.h"
#include "util.h"
#include "atomic.h"


bool init_lfht(lfht_t *ht, uint32_t max){
  uint64_t sz;
  void *addr;
  if(ht != NULL && max != 0){
    sz = (max * sizeof(lfht_entry_t)) + sizeof(lfht_tbl_hdr_t);
    addr = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    
    if (addr != MAP_FAILED) {
      lfht_tbl_hdr_t *hdr = (lfht_tbl_hdr_t *)addr;
      hdr->assimilated = false;
      hdr->sz = sz;
      hdr->max = max;
      hdr->threshold = (uint32_t)(max * RESIZE_RATIO);
      hdr->count = 0;
      hdr->next = NULL;
      hdr->table = addr + sizeof(lfht_tbl_hdr_t);
      ht->table_hdr = hdr;
      return true;
    }
  }
  return false;
}

bool delete_lfht(lfht_t *ht){
  int retcode;
  
  if(ht != NULL && ht->table_hdr != NULL){
    retcode = munmap(ht->table_hdr, ht->table_hdr->sz);
    ht->table_hdr = NULL;
    ht->state = DELETED;
    if(retcode == 0){
      return true;
    }
  }
  return false;
}
 
bool lfht_insert(lfht_t *ht, uintptr_t key, uintptr_t val){
  uint32_t mask, j, i;
  lfht_entry_t*  table;
  lfht_entry_t entry, desired;
  
  if(ht != NULL  && ht->table_hdr != NULL && key != 0){
    table = ht->table_hdr->table;
    desired.key = key;
    desired.val = val;
    mask = ht->table_hdr->max - 1;
    
    j = jenkins_hash_ptr((void *)key) & mask; 
    
    i = j;
    
    while (true) {
      
      entry = table[i];
      
      if(entry.key == key){ break; } 

      if(entry.key == 0){
	if(cas_64((volatile uint64_t *)&(table[i].key), *((uint64_t *)&(entry.key)), *((uint64_t *)&(desired.key)))){
	  //iam: discuss
	  table[i].val = val;
	  return true;
	} else {
	  continue;
	}
      }
      
      i++;
      i &= mask;

      if( i == j ){ break; }

    }
    
  }
  return false;
}

bool lfht_update(lfht_t *ht, uintptr_t key, uintptr_t val){
  uint32_t mask, j, i;
  lfht_entry_t*  table;
  lfht_entry_t entry, desired;
  
  if(ht != NULL  && ht->table_hdr != NULL  && key != 0){
    table = ht->table_hdr->table;
    mask = ht->table_hdr->max - 1;
    desired.key = key;
    desired.val = val;

    j = jenkins_hash_ptr((void *)key) & mask;
    i = j;

    while (true) {

      entry = table[i];

      if(entry.key == key){
	if(cas_64((volatile uint64_t *)&(table[i].val), *((uint64_t *)&(entry.val)), *((uint64_t *)&(desired.val)))){
	  return true;
	} else {
	  continue;
	}
      } else if(entry.key == 0){
	return false;
      }

      i++;
      i &= mask;
      
      if( i == j ){ break; }

    }

  }
	
  return false;

}

bool lfht_insert_or_update(lfht_t *ht, uintptr_t key, uintptr_t val){
  uint32_t mask, j, i;
  lfht_entry_t*  table;
  lfht_entry_t entry, desired;

  if(ht != NULL  && ht->table_hdr != NULL  && key != 0){
    table = ht->table_hdr->table;
    desired.key = key;
    desired.val = val;
    mask = ht->table_hdr->max - 1;
    j = jenkins_hash_ptr((void *)key) & mask;
    i = j;

    while (true) {

      entry = table[i];

      if(entry.key == 0){
	if(cas_64((volatile uint64_t *)&(table[i].key), *((uint64_t *)&(entry.key)), *((uint64_t *)&(desired.key)))){
	  //iam: discuss
	  table[i].val = val;
	  return true;
	} else {
	  continue;
	}
      }
      
      if(entry.key == key){
	if(cas_64((volatile uint64_t *)&(table[i].val), *((uint64_t *)&(entry.val)), *((uint64_t *)&(desired.val)))){
	  return true;
	} else {
	  continue;
	}
      }

      i++;
      i &= mask;
      
      if( i == j ){ break; }

    }
  }
  return false;
}

bool lfht_find(lfht_t *ht, uintptr_t key, uintptr_t *valp){
  uint32_t mask, j, i;
  uint64_t kval;
  lfht_entry_t*  table;
    
  if(ht != NULL  && ht->table_hdr != NULL  && key != 0 && valp != NULL){
    table = ht->table_hdr->table;
    mask = ht->table_hdr->max - 1;
    j = jenkins_hash_ptr((void *)key) & mask;
    i = j;
  

    while (true) {

      kval = read_64((volatile uint64_t *)&table[i].key);

      if(kval == 0){
	return false;
      }

      if(kval == key){
	*valp = table[i].val;
	return true;
      }

      i++;
      i &= mask;
      
      if( i == j ){ break; }
      
    }
  }

  return false;
}
  
