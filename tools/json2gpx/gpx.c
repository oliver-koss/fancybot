#include <stdio.h>
FILE* gpxf;

#define header_1 "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
#define header_2 "<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" version=\"1.1\" creator=\"Wikipedia\"\n    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n    xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n"

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

int main()
{
    gpxf = fopen("./output.gpx", "a");

    fprintf(gpxf, header_1);
    fprintf(gpxf, header_2);
    write_metadata("Test", "Testing some gpx stuff", "Oliver");

    fprintf(gpxf, " <trk>\n  <name>test</name>\n  <trkseg>\n");
    write_trackpoint("47.04765764493451", "15.382984344816208");
    fprintf(gpxf, "  </trkseg>\n  </trk>\n");
    fprintf(gpxf, "</gpx>");
    fclose(gpxf);
    return 0;
}