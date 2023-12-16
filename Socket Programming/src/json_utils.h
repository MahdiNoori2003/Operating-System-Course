#ifndef __JSON_UTILS_H__
#define __JSON_UTILS_H__
#include "define.h"
#include <fcntl.h>
#include <sys/stat.h>
#include "cJSON.h"

#define RECIPES_FILE "src/recipes.json"

typedef struct ingredient
{
    char *name;
    int count;
} ingredient;

typedef struct food
{
    char *name;
    ingredient *ingredients;
    int ingredient_count;
} food;

char *read_file(const char *filename);

char **get_food_names();

char **get_ingridients();

void parse_food(cJSON *foodJson, food *food_struct);

food *get_foods(int *count_of_foods);

#endif