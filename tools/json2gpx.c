#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

FILE* json_log;
FILE* gpx_file;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Invalid input!\n");
        return 1;
    }
    printf("File: %s\n", argv[1]);


    json_log = fopen(argv[1], "r");
    if(json_log == NULL)
    {
        printf("Cannot open file!\n");
        return 1;
    }

    while(true)
    {
        char* buffer = malloc(1000);
        if(buffer == NULL)
        {
            printf("Cannot allocate memory!\n");
            return 1;
        }
 
        if(fgets(buffer, 1000, json_log) == NULL)
        {
            break;
        }

        cJSON* json = cJSON_Parse(buffer);

        /*
        cJSON* name = cJSON_GetObjectItemCaseSensitive(json, "time");
        if (cJSON_IsString(name) && (name->valuestring != NULL)) {
            printf("Name: %s\n", name->valuestring);
        }
        */

        printf("%s\n", buffer);
        cJSON_Delete(json);
        free(buffer);
    }
    fclose(json_log);
    return 0;
}