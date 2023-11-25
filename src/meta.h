#pragma once
#include "zast.h"
#include "hash.h"

static const int SIZE_INT = 4;

typedef enum {
    MT_LET,
} MetaKind;

typedef struct Meta Meta;
struct Meta {
    MetaKind kind;
    char *name;
    int seq;
    int offset;
    Node *node;
};

/**
 * @brief Initializes the meta information.
 * 
 * This function initializes the meta information required for the program.
 * It should be called before any other functions that rely on the meta information.
 */
void init_meta();

Meta *new_meta(Node *expr, MetaKind kind);

/**
 * @brief Sets the meta information for the given object.
 *
 * This function sets the meta information for the object pointed to by the `meta` parameter.
 *
 * @param meta A pointer to the Meta object.
 */
void set_meta(Meta *meta);

/**
 * Retrieves the Meta object associated with the given name.
 *
 * @param name The name of the Meta object to retrieve.
 * @return A pointer to the Meta object, or nullptr if not found.
 */
Meta *get_meta(char *name);

int total_meta_size();
HashTable *get_meta_table();