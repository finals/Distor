#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../rbtree.h"

typedef struct StrUnit {
    rbtree_node_t node;
    uint32_t len;
    char *data;
}str_unit_t;

rbtree_t *rbtree;

void str_unit_rbtree_insert_value(rbtree_node_t *root, rbtree_node_t *node, 
    rbtree_node_t *sentinel)
{
    rbtree_node_t *temp = root;
    str_unit_t *n, *t;
    rbtree_node_t **p;

    n = (str_unit_t *)node;
    for( ; ; ) {
    
        t = (str_unit_t *)temp;
    
        if(node->key != temp->key) {
            p = (node->key > temp->key) ? &temp->right : &temp->left;
        }
        else if(n->len != t->len) {
            p = (n->len > t->len) ? &temp->right : &temp->left;
        }
        else {
            p = (memcmp(n->data, t->data, n->len) > 0) ? 
                &temp->right : &temp->left;
        }

        if(*p == sentinel) 
            break;

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;

    rbt_red(node);
    
}

str_unit_t *str_unit_rbtree_lookup(rbtree_t *tree, str_unit_t *val, 
    uint32_t hash)
{
    int32_t rc;
    str_unit_t *n;
    rbtree_node_t *node, *sentinel;

    node = tree->root;
    sentinel = tree->sentinel;

    while(node != sentinel) {
        n = (str_unit_t *)node;

        if(hash != node->key) {
            node = (hash > node->key) ? node->right : node->left;
            continue;
        }

        if(n->len != val->len) {
            node = (val->len > n->len) ? node->right : node->left;
            continue;
        }

        rc = memcmp(n->data, val->data, val->len);
        if(rc > 0) {
            node = node->left;
            continue;
        }
        if(rc < 0) {
            node = node->right;
            continue;
        }

        return n;
    }

    return NULL;

}

int main()
{
    str_unit_t str[6], *pstr = NULL;

    rbtree = (rbtree_t *)malloc(RbtreeSize);
    rbtree_node_t sentinel;

    rbtree_init(rbtree, &sentinel, str_unit_rbtree_insert_value);

    str[0].data = "abcdef";
    str[0].len = strlen("abcdefg") + 1;
    str[0].node.key = 3;
    rbtree_insert(rbtree, (rbtree_node_t *)&str[0]);

    str[1].data = "qwerty";
    str[1].len = strlen("qwerty") + 1;
    str[1].node.key = 2;
    rbtree_insert(rbtree, (rbtree_node_t *)&str[1]);

    str[2].data = "zxcvb";
    str[2].len = strlen("zxcvb") + 1;
    str[2].node.key = 9;
    rbtree_insert(rbtree, (rbtree_node_t *)&str[2]);

    str[3].data = "poiuy";
    str[3].len = strlen("poiuy") + 1;
    str[3].node.key = 5;
    rbtree_insert(rbtree, (rbtree_node_t *)&str[3]);

    str[4].data = "lkjh";
    str[4].len = strlen("lkjh") + 1;
    str[4].node.key = 1;
    rbtree_insert(rbtree, (rbtree_node_t *)&str[4]);

    pstr = (str_unit_t *)rbtree_min(rbtree->root, &sentinel);

    printf("min len: %u data: %s\n", pstr->len, pstr->data);

    pstr = str_unit_rbtree_lookup(rbtree, &str[3], 5);
    printf("data: %s  key: %u\n", pstr->data, pstr->node.key);

    str[5].data = "lkjh";
    str[5].len = strlen("lkjh") + 1;
    str[5].node.key= 6;
    pstr = str_unit_rbtree_lookup(rbtree, &str[5], 6);
    if(pstr)
        printf("data: %s  key: %u\n", pstr->data, pstr->node.key);
    else
        printf("Not Found\n");
    
    return 0;
}


