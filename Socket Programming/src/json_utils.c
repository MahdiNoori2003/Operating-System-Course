#include "json_utils.h"

char *read_file(const char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("Cannot open file");
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        perror("Cannot get file size");
        close(fd);
        return NULL;
    }

    char *buffer = (char *)malloc(st.st_size + 1);
    if (buffer == NULL)
    {
        perror("Cannot allocate buffer");
        close(fd);
        return NULL;
    }

    ssize_t bytes_read = read(fd, buffer, st.st_size);
    if (bytes_read != st.st_size)
    {
        perror("Cannot read file");
        free(buffer);
        close(fd);
        return NULL;
    }

    buffer[st.st_size] = '\0';

    close(fd);
    return buffer;
}

char **get_food_names()
{
    const char *json = read_file(RECIPES_FILE);
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        printf("Failed to parse JSON.\n");
        return NULL;
    }

    int num_dishes = cJSON_GetArraySize(root);

    char **dish_names = (char **)malloc(num_dishes * sizeof(char *));

    cJSON *dish = root->child;
    int i = 0;
    while (dish != NULL)
    {
        const char *dish_name = dish->string;
        dish_names[i] = strdup(dish_name);
        i++;

        dish = dish->next;
    }

    cJSON_Delete(root);

    return dish_names;
}

char **get_ingridients()
{
    const char *json = read_file(RECIPES_FILE);
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        printf("Failed to parse JSON.\n");
        return NULL;
    }

    int capacity = 10;
    char **ingredients = (char **)malloc(capacity * sizeof(char *));
    int ingredients_count = 0;

    cJSON *dish = root->child;
    while (dish != NULL)
    {
        cJSON *ingredient = dish->child;
        while (ingredient != NULL)
        {
            const char *ingredient_name = ingredient->string;

            int found = 0;
            for (int i = 0; i < ingredients_count; i++)
            {
                if (strcmp(ingredients[i], ingredient_name) == 0)
                {
                    found = 1;
                    break;
                }
            }

            if (!found)
            {
                if (ingredients_count >= capacity)
                {
                    capacity *= 2;
                    char **newDistinctIngredients = (char **)realloc(ingredients, capacity * sizeof(char *));
                    ingredients = newDistinctIngredients;
                }
                ingredients[ingredients_count] = strdup(ingredient_name);
                ingredients_count++;
            }

            ingredient = ingredient->next;
        }

        dish = dish->next;
    }
    cJSON_Delete(root);
    return ingredients;
}

void parse_food(cJSON *foodJson, food *food_struct)
{
    cJSON *ingredient_json = foodJson->child;
    food_struct->name = strdup(foodJson->string);
    food_struct->ingredient_count = cJSON_GetArraySize(foodJson);
    food_struct->ingredients = (ingredient *)malloc(sizeof(ingredient) * food_struct->ingredient_count);

    int i = 0;
    while (ingredient_json)
    {

        food_struct->ingredients[i].name = strdup(ingredient_json->string);
        food_struct->ingredients[i].count = ingredient_json->valueint;
        i++;
        ingredient_json = ingredient_json->next;
    }
}

food *get_foods(int *count_of_foods)
{
    const char *json = read_file(RECIPES_FILE);
    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        printf("Error parsing JSON!\n");
        return NULL;
    }

    int food_count = cJSON_GetArraySize(root);
    food *foods = (food *)malloc(sizeof(food) * food_count);

    int i = 0;
    cJSON *foodJson = root->child;
    while (foodJson)
    {
        parse_food(foodJson, &foods[i]);
        foodJson = foodJson->next;
        i++;
    }
    cJSON_Delete(root);
    *count_of_foods = food_count;
    return foods;
}
