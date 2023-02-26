#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define ALIGNMENT 16   // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))

#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

struct block {
    struct block *next;
    int size;
    int in_use; 
};

void split_block(struct block *curr_block, int requested_size) {
    
    struct block *new_free_node = PTR_OFFSET(curr_block, requested_size + PADDED_SIZE(sizeof(struct block)));
    new_free_node->size = curr_block->size - PADDED_SIZE(sizeof(struct block)) - requested_size;
    new_free_node->next = NULL;
    new_free_node->in_use = 0; 
    curr_block->size = requested_size;
    curr_block->next = new_free_node;

}

struct block *head = NULL;

void myfree(void *p) {
    struct block *free_node = PTR_OFFSET(p, -PADDED_SIZE(sizeof(struct block)));
    free_node->in_use = 0;
    
    struct block *curr_block = head;
    while (curr_block->next != NULL) {
        struct block *next_block = curr_block->next;
        if (curr_block->in_use == 0 && next_block->in_use == 0) {
            curr_block->size+=(next_block->size + PADDED_SIZE(sizeof(struct block)));
            curr_block->next = next_block->next;
        } else {
            curr_block = next_block;
        }
    }
}

void *myalloc(int size) {

    int padded_size = PADDED_SIZE(size);
    int required_space = padded_size + PADDED_SIZE(sizeof(struct block)) + 16;

    if (head == NULL) {
        head = mmap(NULL, 1024, PROT_READ|PROT_WRITE,
                    MAP_ANON|MAP_PRIVATE, -1, 0);
        head->next = NULL;
        head->size = 1024 - PADDED_SIZE(sizeof(struct block));
        head->in_use = 0;
    }

    struct block *curr_block = head;
    while (curr_block != NULL) {
        if (curr_block->size >= padded_size && curr_block->in_use != 1) {
            if (curr_block->size >= required_space) {
                split_block(curr_block, padded_size);
            }
            curr_block->in_use = 1;
            int padded_block_size = PADDED_SIZE(sizeof(struct block));
            return PTR_OFFSET(curr_block, padded_block_size);
        }
        curr_block = curr_block->next;
    }
    return NULL;
}

void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        // Uncomment the following line if you want to see the pointer values
        // printf("[%p:%d,%s]\n", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}

int main(int argc, char *argv[]) {
    void *p, *q, *r, *s;
    
    p = myalloc(10); print_data();
    q = myalloc(20); print_data();
    r = myalloc(30); print_data();
    s = myalloc(40); print_data();
    
    myfree(q); print_data();
    myfree(p); print_data();
    myfree(s); print_data();
    myfree(r); print_data();
}