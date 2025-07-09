#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>

#define header_1 "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
#define header_2 "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" version=\"1.1\" creator=\"Wikipedia\"\n    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n    xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n"

FILE* json_log;
FILE* gpxf;

void write_metadata(char* data_name, char* description, char* author_name)
{
    fprintf(gpxf, " <metadata>\n  <name>");
    fprintf(gpxf, data_name);
    fprintf(gpxf, "</name>\n  <desc>");
    fprintf(gpxf, description);
    fprintf(gpxf, "</desc>\n  <author>\n   <name>");
    fprintf(gpxf, author_name);
    fprintf(gpxf, "</name>\n  </author>\n </metadata>\n");
}

void write_trackpoint(char* lat, char* lon)
{
    fprintf(gpxf, "  <trkpt lat=\"");
    fprintf(gpxf, lat);
    fprintf(gpxf, "\" lon=\"");
    fprintf(gpxf, lon);
    fprintf(gpxf, "\">\n  </trkpt>\n");
}

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

    gpxf = fopen("./output.gpx", "a");

    fprintf(gpxf, header_1);
    fprintf(gpxf, header_2);
    write_metadata("Test", "Testing some gpx stuff", "Oliver");

    fprintf(gpxf, " <trk>\n  <name>test</name>\n  <trkseg>\n");


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

        cJSON* location_array = cJSON_GetObjectItemCaseSensitive(json, "gps");

        cJSON *location = NULL;

        char lat[50];
        char lon[50];

        int count = 0;
        bool error = false;
        cJSON_ArrayForEach(location, location_array) {
            if (count == 0)
            {
                sprintf(lon, "%f", location->valuedouble);
//                printf("Lon: %s\n", lon);
                count++;
            } else {
                sprintf(lat, "%f", location->valuedouble);
//                printf("Lat: %s\n", lat);
            }

            if (location->valuedouble == 0)             // !!!
            {
                error = true;
                break;
            }
        }

        if(!error) {
            write_trackpoint(lat, lon);
        }

//        printf("%s\n", buffer);
        cJSON_Delete(json);
        free(buffer);
    }

    fprintf(gpxf, "  </trkseg>\n  </trk>\n");
    fprintf(gpxf, "</gpx>");
    fclose(gpxf);

    fclose(json_log);
    return 0;
}