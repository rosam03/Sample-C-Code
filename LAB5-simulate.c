/*
 * Edited by: Rosa Mohammadi
 * CPSC 261 Lab 5
 * simulate
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/*
 * Size of a cache data block.
 */
#define LOG2BLOCKSIZE 2
#define BLOCKSIZE (1 << LOG2BLOCKSIZE)

/*
 * N is the number of lines in a set (N-way set associative).
 */
#define LOG2N 3
#define N (1 << LOG2N)

/*
 * NSETS is the number of sets in the cache.
 */
#define LOG2NSETS 2
#define NSETS (1 << LOG2NSETS)

/*
 * ADDRESSWIDTH is the number of bits in an address.
 */
#define ADDRESSWIDTH 8

/*
 * Print more debugging messages.
 */
#define VERBOSE 1   // Turn this off to not print hits

/*
 * A cache line:  a block plus control information.
 * Note that the LRU implementation in this cache in unrealistic
 * in that it assumes that we have enough bits to hold the time
 * of every access
 *
 * PROVIDED BY INSTRUCTOR
 */
typedef struct
{
    int valid;
    unsigned long tag;
    long last_use_order;
    char block[BLOCKSIZE];
} cache_line_t;

/*
 * A cache set, nothing but an indexed collection (array) of lines.
 *
 * PROVIDED BY INSTRUCTOR
 */
typedef struct
{
    cache_line_t *lines[N];
} cache_set_t;

/*
 * A cache, nothing but an indexed collection (array) of sets.
 *
 * PROVIDED BY INSTRUCTOR
 */
typedef struct
{
    cache_set_t *sets[NSETS];
} cache_t;

/*
 * An actual cache.
 *
 * PROVIDED BY INSTRUCTOR
 */
cache_t *cache;

/*
 * Return the tag from the address.
 */
unsigned long extract_tag_from_address(unsigned long address)
{
    unsigned long tag = address; // Bitwise operate on address to return tag bits
    
    // Get offset bits
    int offsetBits = LOG2BLOCKSIZE;
    
    // Get index bits
    int setIndexBits = LOG2NSETS;
    
    // Tag bits are bits left over after subtracting the offset and setindex bits
    int tagBits = ADDRESSWIDTH - offsetBits - setIndexBits;
  
    tag = tag >> (ADDRESSWIDTH - tagBits);  // Shift right to only display tag bits
    
    return tag;
}

/*
 * Return the set index from the address.  This function extracts
 * the set index bits of the address, and shifts them so that they
 * can be used to index into the array of sets in the cache.
 */
unsigned long extract_set_number_from_address(unsigned long address)
{
    
    // Bitwise operate on the address to return set index bits of address
    unsigned long setIndex = address;
    
    // Get offset bits
    int offsetBits = LOG2BLOCKSIZE;

    setIndex = setIndex >> offsetBits; // Shift right to remove offset bits
    
    setIndex = setIndex & 0x03; // Keep only the index bits of 8-bit address

    return setIndex; // Return only the set index bits of address
}

/*
 * Update the time of last use for the line associated with the given
 * tag in the set identfied by the given index.  The parameter "now"
 * is the current time which should be stored in the last_use_order field
 * of the correct cache line.
 */
void update_last_used_order(long set_index, unsigned long tag, long now)
{
            long y = 0; // Variable for loop
    
            for(y = 0; y < N; y++){ // Search the lines in this set
                // If we find the line associated with the given tag
                if(cache->sets[set_index]->lines[y]->tag == tag){
                    // Update the last use order field of this line to now
                    cache->sets[set_index]->lines[y]->last_use_order = now;
                }
            }

}

/*
 * Return the index of the line in the set identified by the given set_index
 * that has been least recently used (LRU).  If any line in the set is invalid,
 * then return the index of that line.  If all lines are valid, then return the 
 * index of the line with the smallest (oldest) last_use_order field.
 */
long get_lru_line_index(long set_index)
{
    long indexLRU = 0;  // The index of the LRU line
    long previous = 0;  // Time of LRU line access
    long d = 0; // Variable for loop 1
    long i = 0; // Variable for loop 2
    
    for(d = 0; d < N; d++){ // Search the lines in this set
        // If the line in this set is invalid
        if(cache->sets[set_index]->lines[d]->valid != 1){
            return d; // Return the index of this line
        }
            previous = cache->sets[set_index]->lines[0]->last_use_order; // Begin with previous as the line[0] access time
    }
    
    for(i = 0; i < N; i++){ // Search the lines in this set
        // If this line has been LRU than the last line searched,
        if(cache->sets[set_index]->lines[i]->last_use_order < previous){
            previous = cache->sets[set_index]->lines[i]->last_use_order; // Update the time to this lines access time
            indexLRU = i; // Save the index of the line in this set
        }
    }

    return indexLRU; // Return the index of the LRU line in the set
    
}

/*
 * If the indicated tag is in one of the cache lines in the set identified
 * by set_index, then return the index of that line.  Return -1 otherwise.
 */
long tag_in_set(long set_index, unsigned long tag)
{
    long s = 0; // Variable for loop
    
    for(s = 0; s < N; s++){ // Search the lines in this set
        if(cache->sets[set_index]->lines[s]->tag == tag){ // If the tag is in this line
            return s; // Return the index of this line
        }
    }
        
    return -1; // If tag is not in one of the lines, return -1
}

/*
 * Add the given tag to the given set.  This function must first identify which
 * line in the set is least recently used (the get_lru_line_index() function determines
 * this).  Then it updates that cache line to record that it is valid, that is 
 * contains this tag, and that the line has been accessed at the current time.
 * This function must return the index of the line in the set that has been updated.
 */
long add_line_to_set(long set_index, unsigned long tag, long now)
{
   
    // Get the LRU line for this set index
    long lineIndexLRU = (long) get_lru_line_index(set_index);
    // Update this lines tag
    cache->sets[set_index]->lines[lineIndexLRU]->tag = tag;
    // Make this line valid
    cache->sets[set_index]->lines[lineIndexLRU]->valid = 1;
    // Update the time it is accessed to time of now
    cache->sets[set_index]->lines[lineIndexLRU]->last_use_order = now;
    
    return lineIndexLRU;    // Return the line that has been updated
}

/*
 * Allocate and initialize the cache structures.  This function is complete.
 */
void init_cache() 
{
    long set_index, lineindex;
    cache = malloc(sizeof(cache_t));
    
    for (set_index = 0; set_index < NSETS; ++set_index)
    {
	cache_set_t *set = malloc(sizeof(cache_set_t));
	cache->sets[set_index] = set;
	for (lineindex = 0; lineindex < N; ++lineindex)
        {
	    cache_line_t *line = malloc(sizeof(cache_line_t));
	    line->valid = 0;
	    set->lines[lineindex] = line;
	}
    }
    printf("The cache has %d sets, each with %d lines, total size = %d bytes\n", NSETS, N, NSETS * N * BLOCKSIZE);
}

/*
 * Read the sequence of addresses and simulate the cache.  This function is complete
 * except that it calls a number of functions that are not.
 *
 * PROVIDED BY INSTRUCTOR
 */
int main(int argc, char **argv)
{
    int i = 0;
    unsigned long addr, tag;
    long set_index, line, accesses = 0, hits = 0, now = 0;

    init_cache();
    srandom(time(0));

    while (!feof(stdin))
    {
	if (1 != scanf("%lX", &addr))
        {
	    break;
	}

	accesses++;
	tag = extract_tag_from_address(addr);
        
	set_index = extract_set_number_from_address(addr);

	if ((line = tag_in_set(set_index, tag)) >= 0)
        {
	    hits ++;
	    if (VERBOSE)
   	    {
		printf("0x%06lx:  Hit in line %ld in set %ld\n", addr, line, set_index);
	    }
	    update_last_used_order(set_index, tag, now);
	} 
	else
	{
	    line = add_line_to_set(set_index, tag, now);
	    if (VERBOSE)
	    {
		printf("0x%06lx:  Replaced line %ld in set %ld\n", addr, line, set_index);
	    }
	}
	i++;
	now++;
    }
    printf("%ld hits in %ld accesses; hit rate = %5.2f%%\n", hits, accesses, (double) hits / accesses * 100.0);
}
